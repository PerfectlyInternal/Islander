#include <internal/texture_loader.h>

/*
Custom function to load a texture from a .bmp (bitmap) file
Takes the file path as a parameter
Returns a GLuint that the texture can be referenced by OpenGL functions
This function is extremely dependent on the formatting of the bmp file, making it very difficult to use
*/
GLuint load_bmp(const char* image_path)
{
	// variables to read the data in the file
	unsigned char header[54]; // 54-byte header at the beginning fo the bmp file
	unsigned int data_pos; // the position where the data begins in the bmp file
	unsigned int width, height; // width and height of the image
	unsigned int image_size; // the size of the image, in bytes

	FILE * file = fopen(image_path, "rb");
	if (!file) printf("Could not open bmp file %s\n", image_path);

	// read the header
	if (fread(header, 1, 54, file) != 54) // if there aren't 54 bytes in the header
	{
		printf("Invalid bmp file\n");
		return 0;
	}

	// check if the bmp file has the correct header beginning
	if (header[0] != 'B' || header[1] != 'M')
	{
		printf("Invalid bmp file\n");
		return 0;
	}

	// read ints from the header byte array
	data_pos = *(int*)&(header[0x0A]);
	image_size = *(int*)&(header[0x22]);
	width = *(int*)&(header[0x12]);
	height = *(int*)&(header[0x16]);

	// if information is missing, try to guess it
	if (image_size == 0) image_size = width * height * 3; // assume the image's size is (amount of pixels) * (3 bytes per pixel)
	if (data_pos == 0) data_pos = 54; // assume data begins at the end of the header

	// read the image data
	unsigned char* data = new unsigned char[image_size]; // manually allocate the required amount of memory for the data
	fread(data, 1, image_size, file);

	// close the file now that we're done reading it
	fclose(file);

	// create the texture
	GLuint texture_id;
	glGenTextures(1, &texture_id);

	glBindTexture(GL_TEXTURE_2D, texture_id);

	// give the image data to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

	// set the interpolation settings
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // use linear interpolation when magnifying the image
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // use mipmaps and linear interpolation when minifying the image
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // repeat image
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	
	glGenerateMipmap(GL_TEXTURE_2D);

	delete data;

	return texture_id;
}

/*
GLI-based image loader
Takes a DDS file as input
Returns a GLuint reference to a texture
Use the Compressonator to make DDS files
*/

#define FOURCC_DXT1 0x31545844 // Equivalent to "DXT1" in ASCII
#define FOURCC_DXT3 0x33545844 // Equivalent to "DXT3" in ASCII
#define FOURCC_DXT5 0x35545844 // Equivalent to "DXT5" in ASCII

GLuint load_dds(const char * imagepath) {

	unsigned char header[124];

	FILE *file;

	/* try to open the file */
	file = fopen(imagepath, "rb");
	if (file == NULL) {
		printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath); getchar();
		return 0;
	}

	/* verify the type of file */
	char filecode[4];
	fread(filecode, 1, 4, file);
	if (strncmp(filecode, "DDS ", 4) != 0) {
		fclose(file);
		return 0;
	}

	/* get the surface desc */
	fread(&header, 124, 1, file);

	unsigned int height = *(unsigned int*)&(header[8]);
	unsigned int width = *(unsigned int*)&(header[12]);
	unsigned int linear_size = *(unsigned int*)&(header[16]);
	unsigned int mipmap_count = *(unsigned int*)&(header[24]);
	unsigned int fourCC = *(unsigned int*)&(header[80]);

	unsigned char * buffer;
	unsigned int bufsize;
	/* how big is it going to be including all mipmaps? */
	bufsize = mipmap_count > 1 ? linear_size * 2 : linear_size;
	buffer = (unsigned char*)malloc(bufsize * sizeof(unsigned char));
	fread(buffer, 1, bufsize, file);
	/* close the file pointer */
	fclose(file);

	unsigned int components = (fourCC == FOURCC_DXT1) ? 3 : 4;
	unsigned int format;
	switch (fourCC)
	{
	case FOURCC_DXT1:
		format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		break;
	case FOURCC_DXT3:
		format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		break;
	case FOURCC_DXT5:
		format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		break;
	default:
		free(buffer);
		return 0;
	}

	// Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	unsigned int blockSize = (format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;
	unsigned int offset = 0;

	/* load the mipmaps */
	for (unsigned int level = 0; level < mipmap_count && (width || height); ++level)
	{
		unsigned int size = ((width + 3) / 4)*((height + 3) / 4)*blockSize;
		glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height,	0, size, buffer + offset);

		offset += size;
		width /= 2;
		height /= 2;

		// Deal with Non-Power-Of-Two textures.
		if (width < 1) width = 1;
		if (height < 1) height = 1;
	}

	free(buffer);

	return textureID;
}
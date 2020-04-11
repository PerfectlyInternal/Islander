#include <internal/texture_loader.h>

/*
Custom function to load a texture from a .bmp (bitmap) file
Takes the file path as a parameter
Returns a GLuint that the texture can be referenced by OpenGL functions
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
	unsigned char* data = (unsigned char*) malloc(image_size); // manually allocate the required amount of memory for the data
	fread(data, 1, image_size, file);

	// close the file now that we're done reading it
	fclose(file);

	// create the texture
	GLuint texture_id;
	glGenTextures(1, &texture_id);

	glBindTexture(GL_TEXTURE_2D, texture_id);

	// give the image data to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB	, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

	// set the interpolation settings
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // use linear interpolation when magnifying the image
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // use mipmaps and linear interpolation when minifying the image
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // repeat image
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glGenerateMipmap(GL_TEXTURE_2D);

	return texture_id;
}
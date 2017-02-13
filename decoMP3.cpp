#include <fstream>
#include <iostream>
#include <cstring>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <bitset>
#include <math.h>

using namespace std;

int countSizeOfTag(int fd)
{//Function finds size of Tag at the begging of the file
	char tagBytes[10];
	bitset<8>sizeOfTag[4];
	string temp;
	char buffer[4][9] = {0};
	char sizeBits[28] = {0};
	long int size = 0;

	lseek(fd, 0, SEEK_SET);
	read(fd, tagBytes,10);

	//Read from 6 to 10 bytes as it is a place where lies the tag size
	for (int i = 6; i < 10;i++)
	{
		sizeOfTag[i-6] = bitset<8>(tagBytes[i]);
		temp = sizeOfTag[i-6].to_string();
		copy(temp.begin() + 1, temp.end(), buffer[i-6]);
	}

	//Puts bits responsible for the size into the string
	for (int i = 0; i < 4;i++)
		strcat(sizeBits, buffer[i]);
	//Count size of tag
	for (int i = strlen(sizeBits); i > 0;i--)
		size += (sizeBits[i-1]-48) * pow(2, strlen(sizeBits) - i);
		
	return size;
}

void findHeader(int fd, long int size)
{
	bitset<8>buf[2];
	bitset<8>oneHeaderByte[4];
	string temp;
	long int step = 0;
	char headerBytes[4] = {0}, headBits[36] = {0}, buffer[4][9] = {0};

	//Seek until find header sync bits
	lseek(fd, size, SEEK_SET);
	do
	{
		if((read(fd,(char*)&buf[0],1) || read(fd,(char*)&buf[1],1)) <= 0)
			break;
		size++;
	}while((buf[0] != 0xFF) && (buf[1] != 0xFB));

	//Read header
	lseek(fd, size-1, SEEK_SET);
	read(fd, headerBytes, 4);
	for (int i = 0; i < 4;i++)
	{
		oneHeaderByte[i] = bitset<8>(headerBytes[i]);
		temp = oneHeaderByte[i].to_string();
		copy(temp.begin(), temp.end(), buffer[i]);
	}
	//Puts header bits into the string
	for (int i = 0; i < 4;i++)
		strcat(headBits, buffer[i]);

	//Find the necessary bits and write the results into the OutputFile
	if (headBits[11] == '1' && headBits[12] == '1')
		cout << "MPEGv1.0\n";
	else if (headBits[11] == '1' && headBits[12] == '0')
		cout << "MPEGv2.0\n";
	else if (headBits[11] == '0' && headBits[12] == '0')
		cout << "MPEGv2.5\n";

	if (headBits[13] == '1' && headBits[14] == '1')
		cout << "layer I\n";
	else if (headBits[13] == '1' && headBits[14] == '0')
		cout << "layer II\n";
	else if (headBits[13] == '0' && headBits[14] == '1')
		cout << "layer III\n";

	return;
}

int main(int argc, char**argv)
{//Finds MPEG version and layer version of the fd
	long int fd, size;
	char tagbuf[3];
	
	//read first 3 bytes into the tagbuf
	if ((fd = open(argv[1], O_RDONLY)) < 0)
		exit(0);
	if (read(fd, tagbuf, sizeof(tagbuf)) < 0)
		exit(0);
	
	//If first three bytes of the file is letters ID3 means that there is a tag v2.* before header
	if ((tagbuf[0] == 'I') && (tagbuf[1] == 'D') && (tagbuf[2] == '3'))
	{		
		size = countSizeOfTag(fd);
		findHeader(fd, size);
	}//Else if there is there is no tag, start function that finds framesync bits and lays out file into components
	else 
		findHeader(fd, 0);
	close(fd);

	return 0;
}






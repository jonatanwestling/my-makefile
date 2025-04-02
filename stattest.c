#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

int main()
{
	struct stat file_info1;
	struct stat file_info2;

	const char *file_path1 = "algoritmen"; // Replace with the path to your file
	const char *file_path2 = "mmake.c";

	if (stat(file_path1, &file_info1) == 0)
	{
		// Successfully obtained file information

		printf("Last Modification Time: %d\n", file_info1.st_mtime);

		// You can access more information using other members of the 'file_info' structure
		// For example, 'st_uid' and 'st_gid' represent the user and group IDs of the owner.
	}
	else
	{
		// Error occurred while obtaining file information
		perror("stat");
	}

	if (stat(file_path2, &file_info2) == 0)
	{
		// Successfully obtained file information

		printf("Last Modification Time: %d\n", file_info2.st_mtime);

		// You can access more information using other members of the 'file_info' structure
		// For example, 'st_uid' and 'st_gid' represent the user and group IDs of the owner.
	}
	else
	{
		// Error occurred while obtaining file information
		perror("stat");
	}

	if (file_info1.st_mtime > file_info2.st_mtime)
	{
		printf("Fil 1 är nyare än fil 2\n");
	}


	return 0;
}
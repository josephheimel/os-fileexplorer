#include <iostream>
#include <string>
#include <dirent.h>
#include <sys/stat.h>

void listDirectory(std::string dirname);

int main(int argc, char **argv)
{
    // if no command line paramater exists, list contents of current working directory
    // else, list contents of directory specified
    std::string dirname = ".";
    if (argc >= 2)
    {
        dirname = argv[1];
    }

    listDirectory(dirname);

    return 0;
}

void listDirectory(std::string dirname)
{
    struct stat info;
    int err = stat(dirname.c_str(), &info);
    if (err == 0 && S_ISDIR(info.st_mode))
    {
        DIR* dir = opendir(dirname.c_str());
        // TODO: modify to be able to print all entries in directory in alphabetical order
        //       in addition to file name, also print file size (or 'directory' if entry is a folder)
        //       Example output:
        //         ls.cpp (693 bytes
        //         my_file.txt (62 bytes)
        //         OS Files (directory)
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            printf("%s\n", entry->d_name);
        }
        closedir(dir);
    }
    else
    {
        fprintf(stderr, "Error: directory '%s' not found\n", dirname.c_str());
    }
}

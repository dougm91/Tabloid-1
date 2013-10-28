//Written by Zev Battad
//The File Handler class will be used to interact with files.
// Subject to change!

#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H

#include <QFile>
#include <QDir>
#include <QString>
#include <QFileInfo>
#include <QDataStream>

class FileHandler
{

public:
    //Constructor
    FileHandler();
    //Attempts to open the requested file in the current_directory
    bool OpenFile(QString file_name);
    //Closes the current_open_file
    bool CloseFile();
    //Creates a new file object with the given file name and type
    //NOTE: The new file is in-memory only until it is saved
    bool CreateFile(QString file_name, QString file_type);
    //Deletes the specified file from the current directory
    bool RemoveFile(QString file_name);
    //Saves the current_open_file
    bool SaveFile();
    //Changes the current_directory to the given directory_path
    bool ChangeDirectory(QString directory_path);

    //Calling read without specifying a length will read the entire file
    int Read(char *buffer);
    //Read at most a specified number of bytes from the open file into the specified buffer
    int Read(char *buffer, int length);

    //Write at most a specified number of bytes from the specified buffer into the open file
    int Write(char *buffer, int length);

    //Accessor for the current_open_file
    //NOTE: Going to keep this as a QFile and have other file classes inherit from QFile
    QFile* CurrentFile();
    //Accessor for the current_directory
    QDir CurrentDirectory(){return current_directory;}
private:
    //The file currently opened by the File Handler
    QFile* current_open_file;
    //The current open file's information
    QFileInfo current_open_file_info;
    //The directory the File Handler is currently in
    QDir current_directory;
    //The original directory of the program
    QDir home_directory;

};

#endif // FILE_HANDLER_H

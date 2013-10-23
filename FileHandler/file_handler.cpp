
#include "file_handler.h"

#include <iostream>

#include <QFile>
#include <QDir>
#include <QString>
#include <QTextStream>

using namespace std;

//Constructor
FileHandler::FileHandler()
{
    current_directory = QDir::currentPath();
    home_directory = "";
    current_open_file = NULL;
    //current_open_file_info = 0;
}

QFile* FileHandler::CurrentFile()
{
    //Debugging check
    if (current_open_file == nullptr)
        cout << "No current open file\n";
    return current_open_file;
}

bool FileHandler::OpenFile(QString file_name)
{
    //Check for existence of file
    if (!QFile::exists(file_name))
    {
        cout << "File " << file_name.toStdString() << " does not exist";
        return false;
    }

    QFile file(file_name);  //(TODO: Create a GenericFile instead)
    //Attempt to open file
    //TODO: Call the GenericFile's open method instead
    if (!file.open(QFile::ReadWrite))   //appropriate open flags here
    {
        cout << "File " << file_name.toStdString() << " could not be opened";
        return false;
    }

    //If there is a file currently open, close it first
    if (current_open_file != NULL)
    {
        CloseFile();
    }

    //Set the current_open_file to the newly opened file
    current_open_file = &file;

    cout << "Current open file: " << current_open_file->fileName().toStdString() << "\n";

    //Retrieve the file's information
    current_open_file_info = QFileInfo(file_name);
    return true;
}

bool FileHandler::CloseFile()
{
    //Check that there is a file currently open
    if (current_open_file == NULL)
    {
        cout << "No file to close\n";
        return false;
    }

    //TODO: Call the current_open_file's own close method instead
    current_open_file->close();
    return true;

}

bool FileHandler::CreateFile(QString file_name, QString file_type)
{
    QFile file(file_name);
    //TODO: Create a GenericFile of appropriate file_type
    //Right now only makes text files
    //NOTE: Calling QFile's open method creates the file
    if (!file.open(QFile::WriteOnly | QFile::Text))
    {
        cout << "Could not create file " << file_name.toStdString() << "\n";
        return false;
    }

    return OpenFile(file_name);
}

bool FileHandler::RemoveFile(QString file_name)
{
    //Check for existence of file
    if (!QFile::exists(file_name))
    {
        cout << "File " << file_name.toStdString() << " does not exist";
        return false;
    }
    //Check that it is not the file currently open
    if (current_open_file != nullptr)
        if (file_name == current_open_file->fileName())
        {
            cout << "File " << file_name.toStdString() << " is currently open";
            return false;
        }

    QFile::remove(file_name);
    return true;
}

bool FileHandler::ChangeDirectory(QString directory_path)
{
    if(!current_directory.cd(directory_path))
    {
        cout << "Changing to directory " << directory_path.toStdString() << " failed\n";
        return false;
    }
    else
    {
        QDir::setCurrent(directory_path);
        return true;
    }
}

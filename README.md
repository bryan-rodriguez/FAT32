# FAT32
C program that allows the user to read FAT32 disk images

Commands include

stat <filename> or <directory name>
This command shall print the attributes and starting cluster number of the file or directory name.  If the parameter is a directory name then the size shall be 0. If the file or
directory does not exist then your program shall output “Error: File not found”.
  
get <filename>
  This command shall retrieve the file from the FAT 32 image and place it in your current working directory.   If the file or directory does not exist then the
  output is “Error: File not found”.
  
cd <directory>
  This command shall change the current working directory to the given directory.  '..' used to go to previous directory
  
ls
  Lists the directory contents.
  
read <filename> <position> <number of bytes>
  Reads from the given file at the position, in bytes, specified by the position parameter and output the number of bytes specified.

#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <string>
#include <dirent.h>
#include <sys/stat.h>
#include <vector>
#include <algorithm>
#include <filesystem>
#include <fstream>


/*
        Icons made by KKing: https://icon-icons.com/users/lUybzhSQf3kZ7FimJzYlO/icon-sets/
        under CC Atribution license: https://creativecommons.org/licenses/by/4.0/
        no changes were made.
*/

#define WIDTH 800
#define HEIGHT 600

using namespace std;
namespace fs = std::filesystem;

typedef struct AppData {
    TTF_Font *font;
    //file name
    std::vector<SDL_Texture*> name;
    std::vector<SDL_Rect> name_coordinates;
    //file icon
    std::vector<SDL_Texture*> icon;
    std::vector<SDL_Rect> icon_coordinates;
    std::vector<int> icon_type;
    //file permissions
    std::vector<std::string> permissions;
    std::vector<SDL_Texture*> permissions_text;
    std::vector<SDL_Rect> permissions_coordinates;
    //file size
    std::vector<std::string> size;
    std::vector<SDL_Texture*> size_text;
    std::vector<SDL_Rect> size_coordinates;

    //header
    SDL_Texture *name_header;
    SDL_Rect name_header_coordinates;
    SDL_Texture *size_header;
    SDL_Rect size_header_coordinates;
    SDL_Texture *permissions_header;
    SDL_Rect permissions_header_coordinates;
    //button
    SDL_Rect recursive_button_outline;
    SDL_Rect recursive_button;
    bool recursive_viewing_mode;

    //scrollbar
    SDL_Rect scrollbar_outline;
    SDL_Rect scrollbar;
    SDL_Point scrollbar_offset;

} AppData;

void initialize(SDL_Renderer *renderer, AppData *data_ptr);
void initializeIcons(SDL_Renderer *renderer, AppData *data_ptr);
void render(SDL_Renderer *renderer, AppData *data_ptr);
void listDirectory(std::string dirname, char ***result, AppData *data_ptr);
bool compareNoCase (std::string first, std::string second);
void cleanTextures(AppData *data_ptr);
void cleanIcons(AppData *data_ptr);
std::string getPermissions(fs::perms p);

int main(int argc, char **argv)
{
    char *home = getenv("HOME");
    printf("HOME: %s\n", home);

    // initializing SDL as Video
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();

    // create window and renderer
    SDL_Renderer *renderer;
    SDL_Window *window;
    SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, 0, &window, &renderer);

    // initialize
    AppData data;
    initialize(renderer, &data);
    initializeIcons(renderer, &data);

    //render
    render(renderer, &data);

    //run rendering loop
    SDL_Event event;
    SDL_WaitEvent(&event);
    while (event.type != SDL_QUIT)
    {
        //render(renderer);
        SDL_WaitEvent(&event);
        switch (event.type)
        {
        case SDL_MOUSEMOTION:
            break;
        
        case SDL_MOUSEBUTTONDOWN:
            //tests cleanTextures (works)
            //cleanTextures(&data);
            break;
        
        case SDL_MOUSEBUTTONUP:
            break;
        default:
            break;
        }

        render(renderer, &data);
    }

    // clean up
    cleanTextures(&data);
    cleanIcons(&data);
    TTF_CloseFont(data.font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();

    return 0;
}

void initialize(SDL_Renderer *renderer, AppData *data_ptr)
{
    // set color of background when erasing frame
    SDL_SetRenderDrawColor(renderer, 235, 235, 235, 255);

    //load font
    data_ptr->font = TTF_OpenFont("resrc/OpenSans-Regular.ttf", 18);


    //Create Header

    //Create Recursive Mode Button

    //Create Scrollbar
    int scrollbar_width = 25;
    data_ptr->scrollbar_outline = {2,27,scrollbar_width-4,571};
    //get scrollbar size based on how many files there are

    //Get Home Directory files
    std::string homedir = getenv("HOME");
    char **files;
    listDirectory(homedir, &files, data_ptr);

    int y_offset = 25;
    int icon_side_length;
    int icon_gap = 2;
    for(int i = 0; files[i] != NULL; i++)
    {
        //For each folder in directory: create text, permissions, size, and position variables
        SDL_Texture *text;
        SDL_Rect icon_pos;
        SDL_Rect text_pos;
        SDL_Texture *size;
        SDL_Texture *permissions;
        data_ptr->name_coordinates.push_back(text_pos);
        data_ptr->name.push_back(text);
        data_ptr->icon_coordinates.push_back(icon_pos);
        data_ptr->size_text.push_back(size);
        data_ptr->permissions_text.push_back(permissions);

        // create text name
        SDL_Color phrase_color = { 0, 0, 0 };
        SDL_Surface *text_surf = TTF_RenderText_Solid(data_ptr->font, files[i], phrase_color);
        data_ptr->name.at(i) = SDL_CreateTextureFromSurface(renderer, text_surf);
        SDL_FreeSurface(text_surf);
        data_ptr->name_coordinates.at(i).x = scrollbar_width + 25;
        data_ptr->name_coordinates.at(i).y = y_offset;
        SDL_QueryTexture(data_ptr->name.at(i), NULL, NULL, &(data_ptr->name_coordinates.at(i).w), &(data_ptr->name_coordinates.at(i).h));

        icon_side_length = data_ptr->name_coordinates.at(i).h - icon_gap*2;

        //set icon dimensions
        data_ptr->icon_coordinates.at(i).x = scrollbar_width + icon_gap;
        data_ptr->icon_coordinates.at(i).y = y_offset + icon_gap;
        data_ptr->icon_coordinates.at(i).w = icon_side_length;
        data_ptr->icon_coordinates.at(i).h = icon_side_length;

        y_offset += data_ptr->name_coordinates.at(i).h;
    }
}

void render(SDL_Renderer *renderer, AppData *data_ptr)
{
    // erase renderer content
    SDL_SetRenderDrawColor(renderer, 235, 235, 235, 255);
    SDL_RenderClear(renderer);
    
    //Draw
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &data_ptr->scrollbar_outline);
    
    for(int i = 0; i < data_ptr->name.size(); i++) {
        //determine correct folder type
        SDL_RenderCopy(renderer, data_ptr->icon.at(data_ptr->icon_type.at(i)), NULL, &(data_ptr->icon_coordinates.at(i)));
        SDL_RenderCopy(renderer, data_ptr->name.at(i), NULL, &(data_ptr->name_coordinates.at(i)));
    }


    // show rendered frame
    SDL_RenderPresent(renderer);
}

void cleanTextures(AppData *data_ptr)
{
    for(int i = 0; i < data_ptr->name.size(); i++)
    {
        //file data textures
        SDL_DestroyTexture(data_ptr->name.at(i));
        SDL_DestroyTexture(data_ptr->permissions_text.at(i));
        SDL_DestroyTexture(data_ptr->size_text.at(i));
    }
    //header
    SDL_DestroyTexture(data_ptr->size_header);
    SDL_DestroyTexture(data_ptr->name_header);
    SDL_DestroyTexture(data_ptr->permissions_header);

    //data vectors
    data_ptr->name.clear();
    data_ptr->icon_type.clear();
    data_ptr->permissions.clear();
    data_ptr->size.clear();
    //coordinate vectors
    data_ptr->icon_coordinates.clear();
    data_ptr->name_coordinates.clear();
    data_ptr->size_coordinates.clear();
    data_ptr->permissions_coordinates.clear();
}

void cleanIcons(AppData *data_ptr)
{
    SDL_DestroyTexture(data_ptr->icon.at(0));
    SDL_DestroyTexture(data_ptr->icon.at(1));
    SDL_DestroyTexture(data_ptr->icon.at(2));
    SDL_DestroyTexture(data_ptr->icon.at(3));
    SDL_DestroyTexture(data_ptr->icon.at(4));
    SDL_DestroyTexture(data_ptr->icon.at(5));
}

void initializeIcons(SDL_Renderer *renderer, AppData *data_ptr)
{
    //Directory: index 0 in icon vector
    SDL_Surface *surf = IMG_Load("resrc/images/directory.png");
    SDL_Texture *icon = SDL_CreateTextureFromSurface(renderer, surf);
    data_ptr->icon.push_back(icon);

    //Executable: index 1 in icon vector
    surf = IMG_Load("resrc/images/executable.png");
    icon = SDL_CreateTextureFromSurface(renderer, surf);
    data_ptr->icon.push_back(icon);

    //Image: index 2 in icon vector
    surf = IMG_Load("resrc/images/image.png");
    icon = SDL_CreateTextureFromSurface(renderer, surf);
    data_ptr->icon.push_back(icon);

    //Video: index 3 in icon vector
    surf = IMG_Load("resrc/images/video.png");
    icon = SDL_CreateTextureFromSurface(renderer, surf);
    data_ptr->icon.push_back(icon);

    //Code File: index 4 in icon vector
    surf = IMG_Load("resrc/images/code_file.png");
    icon = SDL_CreateTextureFromSurface(renderer, surf);
    data_ptr->icon.push_back(icon);

    //Other: index 5 in icon vector
    surf = IMG_Load("resrc/images/other.png");
    icon = SDL_CreateTextureFromSurface(renderer, surf);
    data_ptr->icon.push_back(icon);

    SDL_FreeSurface(surf);
}

void listDirectory(std::string dirname, char ***result, AppData *data_ptr)
{
    struct stat info;

    std::vector<std::string> files;
    
    int err = stat(dirname.c_str(), &info);
    if (err == 0 && S_ISDIR(info.st_mode))
    {
        DIR* dir = opendir(dirname.c_str());

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {      
            files.push_back(entry->d_name);
            if(files.back() == "."){
                files.pop_back();
            }
        }
        closedir(dir);
    }
    else
    {
        fprintf(stderr, "Error: directory '%s' not found\n", dirname.c_str());
    }

    //sort
    std::sort(files.begin(), files.end(), compareNoCase);


    //set file types, permissions, and sizes
    for(int i = 0; i < files.size(); i++)
    {
        //file path/name
        fs::path fp = dirname + "/" + files.at(i);
        std::string file = files.at(i);


        //permissions
        data_ptr->permissions.push_back(getPermissions(fs::status(fp).permissions()));

        //size
        std::string bytes = "-";
        if(!fs::is_directory(fp)){
            int size = fs::file_size(fp);
            if(size < 1024){
                bytes = size + " B";
            } else if(size < 1048567) {
                bytes = size/1024 + " KiB";
            } else if(size < 1073741824) {
                bytes = size/1048567 + " MiB";
            } else {
                bytes = size/1073741824 + " GiB";
            }
        }
        data_ptr->size.push_back(bytes);


        //file type
        if(fs::is_directory(fp)) {//file is a directory, icon array index 0
            data_ptr->icon_type.push_back(0);
        } else if (((fs::status(fp).permissions() & fs::perms::owner_exec) != fs::perms::none) ||
                   ((fs::status(fp).permissions() & fs::perms::group_exec) != fs::perms::none) ||
                   ((fs::status(fp).permissions() & fs::perms::others_exec) != fs::perms::none)) {//file is an executable, icon array index 1
            data_ptr->icon_type.push_back(1);
        } else if ((file.find(".jpg") != std::string::npos) ||
                    (file.find(".jpeg") != std::string::npos) ||
                    (file.find(".png") != std::string::npos) ||
                    (file.find(".tif") != std::string::npos) ||
                    (file.find(".tiff") != std::string::npos) ||
                    (file.find(".gif") != std::string::npos)) {//file is an image, icon array index 2
            data_ptr->icon_type.push_back(2);
        } else if ((file.find(".mp4") != std::string::npos) ||
                    (file.find(".mov") != std::string::npos) ||
                    (file.find(".mkv") != std::string::npos) ||
                    (file.find(".avi") != std::string::npos) ||
                    (file.find(".webm") != std::string::npos)) {//file is a video, icon array index 3
            data_ptr->icon_type.push_back(3);
        } else if ((file.find(".h") != std::string::npos) ||
                    (file.find(".c") != std::string::npos) ||
                    (file.find(".cpp") != std::string::npos) ||
                    (file.find(".py") != std::string::npos) ||
                    (file.find(".java") != std::string::npos) ||
                    (file.find(".js") != std::string::npos)) {//file is a code file, icon array index 4
            data_ptr->icon_type.push_back(4);
        } else {//file is other, icon array index 5
            data_ptr->icon_type.push_back(5);
        }
    }

    //convert to char array
    int i;
    int result_length = files.size() + 1;
    *result = new char*[result_length];
    for (i = 0; i < files.size(); i++)
    {
        (*result)[i] = new char[files[i].length() + 1];
        strcpy((*result)[i], files[i].c_str());
    }
    (*result)[files.size()] = NULL;
}

std::string getPermissions(fs::perms p)
{
    std::string ret = "";

    //owner
    if((p & fs::perms::owner_read) != fs::perms::none){
        ret += "r";
    } else {
        ret += "-";
    }
    if((p & fs::perms::owner_write) != fs::perms::none){
        ret += "w";
    } else {
        ret += "-";
    }
    if((p & fs::perms::owner_exec) != fs::perms::none){
        ret += "x";
    } else {
        ret += "-";
    }
    //group
    if((p & fs::perms::group_read) != fs::perms::none){
        ret += "r";
    } else {
        ret += "-";
    }
    if((p & fs::perms::group_write) != fs::perms::none){
        ret += "w";
    } else {
        ret += "-";
    }
    if((p & fs::perms::group_exec) != fs::perms::none){
        ret += "x";
    } else {
        ret += "-";
    }
    //others
    if((p & fs::perms::others_read) != fs::perms::none){
        ret += "r";
    } else {
        ret += "-";
    }
    if((p & fs::perms::others_write) != fs::perms::none){
        ret += "w";
    } else {
        ret += "-";
    }
    if((p & fs::perms::others_exec) != fs::perms::none){
        ret += "x";
    } else {
        ret += "-";
    }
    return ret;
}

bool compareNoCase (std::string first, std::string second)
{
  int i=0;
  while ((i < first.length()) && (i < second.length()))
  {
    if (tolower (first[i]) < tolower (second[i])) return true;
    else if (tolower (first[i]) > tolower (second[i])) return false;
    i++;
  }

  if (first.length() < second.length()) return true;
  else return false;
}
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
#include <unistd.h>


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
    std::vector<std::string> name;
    std::vector<SDL_Texture*> name_text;
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
    SDL_Rect header_box;
    SDL_Texture *name_header;
    SDL_Rect name_header_coordinates;
    SDL_Texture *size_header;
    SDL_Rect size_header_coordinates;
    SDL_Texture *permissions_header;
    SDL_Rect permissions_header_coordinates;
    //button
    SDL_Texture *button_text;
    SDL_Rect button_text_coordinates;
    SDL_Rect recursive_button_outline;
    SDL_Rect recursive_button;
    bool recursive_viewing_mode;

    //scrollbar
    SDL_Rect scrollbar_outline;
    SDL_Rect scrollbar;
    SDL_Point scrollbar_offset;
    bool scrollbar_selected;

    //general y offset vector
    std::vector<SDL_Point> y_origin;

    //current directory
    std::string directory;

} AppData;

void initialize(SDL_Renderer *renderer, AppData *data_ptr);
void initializeIcons(SDL_Renderer *renderer, AppData *data_ptr);
void render(SDL_Renderer *renderer, AppData *data_ptr);
void getFileData(std::string dirname, AppData *data_ptr);
std::vector<std::string> listDirectory(std::string dirname, bool recurse);
bool compareNoCase (std::string first, std::string second);
void cleanTextures(AppData *data_ptr);
void cleanIcons(AppData *data_ptr);
std::string getPermissions(fs::perms p);
int slashCount(std::string path);

int main(int argc, char **argv)
{
    std::string home = getenv("HOME");
    std::cout << "HOME: " << home << std::endl;

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
    data.recursive_viewing_mode = false;
    data.directory = home;
    initialize(renderer, &data);
    initializeIcons(renderer, &data);

    //scrollbar vars
    bool moving = false;
    int motion_root = 0;

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
            if(data.scrollbar_selected){
                data.scrollbar.y = event.motion.y - data.scrollbar_offset.y;
                
                //change icon/file/size/permissions y positions
                if(moving) {
                    for(int i = 0; i < data.name_coordinates.size(); i++)
                    {
                        int pos = data.y_origin[i].y - (event.motion.y - motion_root)*8;
                        data.name_coordinates[i].y = pos;
                        data.icon_coordinates[i].y = pos;
                        data.permissions_coordinates[i].y = pos;
                        data.size_coordinates[i].y = pos;
                    }
                }
            }
            if(data.scrollbar.y < 32){
                data.scrollbar.y = 32;
                moving = false;
            } else if(data.scrollbar.y + data.scrollbar.h > 593){
                data.scrollbar.y = 593-data.scrollbar.h;
                moving = false;
            } else {
                moving = true;
            }
            break;
        
        case SDL_MOUSEBUTTONDOWN:
            //Scrollbar
            if (event.button.button == SDL_BUTTON_LEFT &&
                event.button.x >= data.scrollbar.x &&
                event.button.x <= data.scrollbar.x + data.scrollbar.w &&
                event.button.y >= data.scrollbar.y &&
                event.button.y <= data.scrollbar.y + data.scrollbar.h)
            {
                data.scrollbar_selected = true;
                data.scrollbar_offset.y = event.button.y - data.scrollbar.y;

                motion_root = event.motion.y;

                //set general y offset
                for(int i = 0; i < data.name_coordinates.size(); i++)
                {
                    data.y_origin[i].y = data.name_coordinates[i].y;
                }
                moving = true;
                SDL_CaptureMouse(SDL_TRUE);
            }
            //Recursive Button
            if (event.button.button == SDL_BUTTON_LEFT &&
                event.button.x >= data.recursive_button_outline.x &&
                event.button.x <= data.recursive_button_outline.x + data.recursive_button_outline.w &&
                event.button.y >= data.recursive_button_outline.y &&
                event.button.y <= data.recursive_button_outline.y + data.recursive_button_outline.h)
            {
                data.recursive_viewing_mode = !data.recursive_viewing_mode;
                cleanTextures(&data);
                initialize(renderer, &data);
            }
            //Select File or Directory
            for(int i = 0; i < data.name_text.size(); i++) 
            {
                if (event.button.button == SDL_BUTTON_LEFT &&
                    //name click
                    ((event.button.x >= data.name_coordinates[i].x &&
                    event.button.x <= data.name_coordinates[i].x + data.name_coordinates[i].w &&
                    event.button.y >= data.name_coordinates[i].y &&
                    event.button.y <= data.name_coordinates[i].y + data.name_coordinates[i].h) ||
                    //icon click
                    (event.button.x >= data.icon_coordinates[i].x &&
                    event.button.x <= data.icon_coordinates[i].x + data.icon_coordinates[i].w &&
                    event.button.y >= data.icon_coordinates[i].y &&
                    event.button.y <= data.icon_coordinates[i].y + data.icon_coordinates[i].h)))
                {
                    //if [i] is a directory
                    if(data.icon_type[i] == 0)
                    {
                        data.directory = data.name[i];
                        cleanTextures(&data);
                        initialize(renderer, &data);
                    }
                    else //[i] is a not a directory
                    {
                                                                        //CODE FOR OPENING FILES HERE
                        //file type can be determined by its icon type as shown in the above *if* statement
                        //0 is directory, 1 is executable, 2 is image, 3 is video, 4 is code file, and 5 is other
                        //data.name vector contains file paths

                        int pid = fork();

                        // child command opens file
                        if(pid == 0)
                        {
                            char *pathstr = new char[data.name[i].length() + 1];
                            strcpy(pathstr, data.name[i].c_str());

                            char *xdgstr = new char[9];
                            strcpy(xdgstr, "xdg-open");

                            char *passes[3] = {xdgstr, pathstr, NULL};

                            execvp(xdgstr, passes);
                        }
                        // parent continues running file application
                    }
                }
            }
            break;
        
        case SDL_MOUSEBUTTONUP:
            data.scrollbar_selected = false;
            SDL_CaptureMouse(SDL_FALSE);
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
    data_ptr->header_box = {0,0,800,25};
    SDL_Color phrase_color = { 0, 0, 0 };
    //Set text
    SDL_Surface *header_surf = TTF_RenderText_Solid(data_ptr->font, "Name", phrase_color);
    data_ptr->name_header = SDL_CreateTextureFromSurface(renderer, header_surf);
    header_surf = TTF_RenderText_Solid(data_ptr->font, "Size", phrase_color);
    data_ptr->size_header = SDL_CreateTextureFromSurface(renderer, header_surf);
    header_surf = TTF_RenderText_Solid(data_ptr->font, "Permissions", phrase_color);
    data_ptr->permissions_header = SDL_CreateTextureFromSurface(renderer, header_surf);
    SDL_FreeSurface(header_surf);
    //Set coordinates
    int size_pos_x = 500;
    int permissions_pos_x = 570;
    data_ptr->name_header_coordinates.x = 50;
    data_ptr->name_header_coordinates.y = 0;
    SDL_QueryTexture(data_ptr->name_header, NULL, NULL, &(data_ptr->name_header_coordinates.w), &(data_ptr->name_header_coordinates.h));
    data_ptr->size_header_coordinates.x = size_pos_x;
    data_ptr->size_header_coordinates.y = 0;
    SDL_QueryTexture(data_ptr->size_header, NULL, NULL, &(data_ptr->size_header_coordinates.w), &(data_ptr->size_header_coordinates.h));
    data_ptr->permissions_header_coordinates.x = permissions_pos_x;
    data_ptr->permissions_header_coordinates.y = 0;
    SDL_QueryTexture(data_ptr->permissions_header, NULL, NULL, &(data_ptr->permissions_header_coordinates.w), &(data_ptr->permissions_header_coordinates.h));

    //Create Recursive Mode Button
    data_ptr->recursive_button_outline = {778,3,19,19};
    data_ptr->recursive_button = {782,7,11,11};
    SDL_Surface *button_surf = TTF_RenderText_Solid(data_ptr->font, "All Files:", phrase_color);
    data_ptr->button_text = SDL_CreateTextureFromSurface(renderer, button_surf);
    SDL_FreeSurface(button_surf);
    data_ptr->button_text_coordinates.x = 705;
    data_ptr->button_text_coordinates.y = 0;
    SDL_QueryTexture(data_ptr->button_text, NULL, NULL, &(data_ptr->button_text_coordinates.w), &(data_ptr->button_text_coordinates.h));


    //Get Directory files
    std::string dir = data_ptr->directory;
    int slash_count = slashCount(dir);
    getFileData(dir, data_ptr);


    //y_origin here is not associated with the data field in AppData
    int y_origin = 25;
    int icon_side_length;
    int icon_gap = 2;
    for(int i = 0; i < data_ptr->name.size(); i++)
    {
        //For each folder in directory: create text, permissions, size, and position variables
        SDL_Texture *text;
        SDL_Rect icon_pos;
        SDL_Rect text_pos;
        SDL_Texture *size;
        SDL_Rect size_pos;
        SDL_Texture *permissions;
        SDL_Rect perm_pos;
        data_ptr->name_coordinates.push_back(text_pos);
        data_ptr->name_text.push_back(text);
        data_ptr->icon_coordinates.push_back(icon_pos);
        data_ptr->size_text.push_back(size);
        data_ptr->size_coordinates.push_back(size_pos);
        data_ptr->permissions_text.push_back(permissions);
        data_ptr->permissions_coordinates.push_back(perm_pos);

        //get file name
        fs::path fp = data_ptr->name.at(i);
        std::string name = fp.filename();

        //Set text
        SDL_Surface *text_surf = TTF_RenderText_Solid(data_ptr->font, name.c_str(), phrase_color);
        data_ptr->name_text.at(i) = SDL_CreateTextureFromSurface(renderer, text_surf);
        text_surf = TTF_RenderText_Solid(data_ptr->font, data_ptr->size.at(i).c_str(), phrase_color);
        data_ptr->size_text.at(i) = SDL_CreateTextureFromSurface(renderer, text_surf);
        text_surf = TTF_RenderText_Solid(data_ptr->font, data_ptr->permissions.at(i).c_str(), phrase_color);
        data_ptr->permissions_text.at(i) = SDL_CreateTextureFromSurface(renderer, text_surf);
        SDL_FreeSurface(text_surf);
        
        //Set Coordinates
        int indents = slashCount(data_ptr->name.at(i)) - slash_count - 1;

        data_ptr->name_coordinates.at(i).x = 50 + (25*indents);
        data_ptr->name_coordinates.at(i).y = y_origin;
        SDL_QueryTexture(data_ptr->name_text.at(i), NULL, NULL, &(data_ptr->name_coordinates.at(i).w), &(data_ptr->name_coordinates.at(i).h));
        data_ptr->size_coordinates.at(i).x = size_pos_x;
        data_ptr->size_coordinates.at(i).y = y_origin;
        SDL_QueryTexture(data_ptr->size_text.at(i), NULL, NULL, &(data_ptr->size_coordinates.at(i).w), &(data_ptr->size_coordinates.at(i).h));
        data_ptr->permissions_coordinates.at(i).x = permissions_pos_x;
        data_ptr->permissions_coordinates.at(i).y = y_origin;
        SDL_QueryTexture(data_ptr->permissions_text.at(i), NULL, NULL, &(data_ptr->permissions_coordinates.at(i).w), &(data_ptr->permissions_coordinates.at(i).h));

        icon_side_length = data_ptr->name_coordinates.at(i).h - icon_gap*2;

        //set icon dimensions
        data_ptr->icon_coordinates.at(i).x = 25 + icon_gap;
        data_ptr->icon_coordinates.at(i).y = y_origin + icon_gap;
        data_ptr->icon_coordinates.at(i).w = icon_side_length;
        data_ptr->icon_coordinates.at(i).h = icon_side_length;

        y_origin += data_ptr->name_coordinates.at(i).h;
    }


    //Create Scrollbar [23 items fit on a page]
    data_ptr->scrollbar_outline = {2,27,21,571};
    int items = data_ptr->name_text.size();
    data_ptr->scrollbar_selected = false;
    //populate y offset vector
    for(int i = 0; i < data_ptr->name_coordinates.size(); i++)
    {
        SDL_Point point;
        data_ptr->y_origin.push_back(point);
    }

    //get scrollbar size based on how many files there are
    if(items < 23) {
        data_ptr->scrollbar = {7,32,11,561};
    } else {
        int height = (561*23)/(items);
        data_ptr->scrollbar = {7,32,11,height};
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
    SDL_RenderFillRect(renderer, &data_ptr->scrollbar);

    
    
    for(int i = 0; i < data_ptr->name_text.size(); i++) {
        //determine correct folder type
        SDL_RenderCopy(renderer, data_ptr->icon.at(data_ptr->icon_type.at(i)), NULL, &(data_ptr->icon_coordinates.at(i)));
        SDL_RenderCopy(renderer, data_ptr->name_text.at(i), NULL, &(data_ptr->name_coordinates.at(i)));
        SDL_RenderCopy(renderer, data_ptr->size_text.at(i), NULL, &(data_ptr->size_coordinates.at(i)));
        SDL_RenderCopy(renderer, data_ptr->permissions_text.at(i), NULL, &(data_ptr->permissions_coordinates.at(i)));
    }

    //Header
    SDL_SetRenderDrawColor(renderer, 235, 235, 235, 255);
    SDL_RenderFillRect(renderer, &data_ptr->header_box);
    SDL_RenderCopy(renderer, data_ptr->name_header, NULL, &(data_ptr->name_header_coordinates));
    SDL_RenderCopy(renderer, data_ptr->size_header, NULL, &(data_ptr->size_header_coordinates));
    SDL_RenderCopy(renderer, data_ptr->permissions_header, NULL, &(data_ptr->permissions_header_coordinates));

    //Recursive Button
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &data_ptr->recursive_button_outline);
    if(data_ptr->recursive_viewing_mode) {
        SDL_RenderFillRect(renderer, &data_ptr->recursive_button);
    }
    SDL_RenderCopy(renderer, data_ptr->button_text, NULL, &(data_ptr->button_text_coordinates));

    // show rendered frame
    SDL_RenderPresent(renderer);
}

void cleanTextures(AppData *data_ptr)
{
    for(int i = 0; i < data_ptr->name_text.size(); i++)
    {
        //file data textures
        SDL_DestroyTexture(data_ptr->name_text.at(i));
        SDL_DestroyTexture(data_ptr->permissions_text.at(i));
        SDL_DestroyTexture(data_ptr->size_text.at(i));
    }
    //header
    SDL_DestroyTexture(data_ptr->size_header);
    SDL_DestroyTexture(data_ptr->name_header);
    SDL_DestroyTexture(data_ptr->permissions_header);
    SDL_DestroyTexture(data_ptr->button_text);

    //data vectors
    data_ptr->name.clear();
    data_ptr->name_text.clear();
    data_ptr->icon_type.clear();
    data_ptr->permissions.clear();
    data_ptr->size.clear();
    data_ptr->y_origin.clear();
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

void getFileData(std::string dirname, AppData *data_ptr)
{
    //set file names
    data_ptr->name = listDirectory(dirname, data_ptr->recursive_viewing_mode);

    data_ptr->name.insert(data_ptr->name.begin(), dirname + "/..");

    //set file types, permissions, and sizes
    for(int i = 0; i < data_ptr->name.size(); i++)
    {
        //file path/name
        fs::path fp = data_ptr->name.at(i);
        std::string file = fp.filename();     


        //permissions
        data_ptr->permissions.push_back(getPermissions(fs::status(fp).permissions()));

        //size
        std::string bytes = "-";
        if(!fs::is_directory(fp)){
            int size = fs::file_size(fp);
            if(size < 1024){
                bytes = std::to_string(size) + " B";
            } else if(size < 1048567) {
                size = size/1024;
                bytes = std::to_string(size) + " KiB";
            } else if(size < 1073741824) {
                size = size/1048567;
                bytes = std::to_string(size) + " MiB";
            } else {
                size = size/1073741824;
                bytes = std::to_string(size) + " GiB";
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
}

std::vector<std::string> listDirectory(std::string dirname, bool recurse)
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
            if(files.back() == "." || files.back() == ".."){
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

    //std::cout << files.size() << std::endl;
    for(int i = 0; i < files.size(); i++)
    { 
        files.at(i) = dirname + "/" + files.at(i);
    }

    for(int i = 0; i < files.size(); i++)
    {        
        fs::path fp = files.at(i);

        std::string file = fp.filename();

        //find subDirectories if recursive viewing is active
        if(fs::is_directory(fp) && recurse && file.at(0) != '.')
        {
            //std::cout << i << std::endl;
            std::vector<std::string> subDirectory = listDirectory(files.at(i), false);
            files.insert(files.begin() + i + 1, subDirectory.begin(), subDirectory.end());
        }
    }

    return files;
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

int slashCount(std::string path)
{
    char slash = '/';
    int slash_count = 0;

    for (int i = 0; (i = path.find(slash, i)) != std::string::npos; i++) {
        slash_count++;
    }

    return slash_count;
}
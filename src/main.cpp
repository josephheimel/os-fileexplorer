#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <string>
#include <dirent.h>
#include <sys/stat.h>
#include <vector>
#include <algorithm>


#define WIDTH 800
#define HEIGHT 600

typedef struct AppData {
    TTF_Font *font;
    SDL_Texture *phrase;
    SDL_Texture *penguin;
    SDL_Rect penguin_location;
    SDL_Rect phrase_location;
    bool penguin_selected;
    bool phrase_selected;
    SDL_Point penguin_offset;
    SDL_Point phrase_offset;

} AppData;

void initialize(SDL_Renderer *renderer, AppData *data_ptr);
void render(SDL_Renderer *renderer, AppData *data_ptr);
void listDirectory(std::string dirname, char ***result);
bool compareNoCase (std::string first, std::string second);


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

    // initialize and perform rendering loop
    AppData data;
    initialize(renderer, &data);
    render(renderer, &data);
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
            if(event.button.x >= data.phrase_location.x && 
               event.button.x <= data.phrase_location.x + data.phrase_location.w &&
               event.button.y >= data.phrase_location.y && 
               event.button.y <= data.phrase_location.y + data.phrase_location.h)
            {
                data.phrase_selected = true;
                data.phrase_offset.x = event.button.x - data.phrase_location.x;
                data.phrase_offset.y = event.button.y - data.phrase_location.y;
            } 
            else if(event.button.x >= data.penguin_location.x && 
               event.button.x <= data.penguin_location.x + data.penguin_location.w &&
               event.button.y >= data.penguin_location.y && 
               event.button.y <= data.penguin_location.y + data.penguin_location.h)
            {
                data.penguin_selected = true;
                data.penguin_offset.x = event.button.x - data.penguin_location.x;
                data.penguin_offset.y = event.button.y - data.penguin_location.y;
            }
            break;
        
        case SDL_MOUSEBUTTONUP:
            data.penguin_selected = false;
            data.phrase_selected = false;
            break;
        default:
            break;
        }

        render(renderer, &data);
    }

    // clean up
    SDL_DestroyTexture(data.penguin);
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

    //load linux penguin image
    SDL_Surface *surf = IMG_Load("resrc/images/linux-penguin.png");
    data_ptr->penguin = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    data_ptr->penguin_location.x = 200;
    data_ptr->penguin_location.y = 150;
    data_ptr->penguin_location.w = 165;
    data_ptr->penguin_location.h = 200;
    data_ptr->penguin_selected = false;


    //Get Home Directory files
    std::string homedir = getenv("HOME");
    char **files;
    listDirectory(homedir, &files);

    // create text phrase
    SDL_Color phrase_color = { 0, 0, 0 };
    SDL_Surface *text_surf = TTF_RenderText_Solid(data_ptr->font, files[0], phrase_color);
    data_ptr->phrase = SDL_CreateTextureFromSurface(renderer, text_surf);
    SDL_FreeSurface(text_surf);
    data_ptr->phrase_selected = false;
    data_ptr->phrase_location.x = 10;
    data_ptr->phrase_location.y = 500;
    SDL_QueryTexture(data_ptr->phrase, NULL, NULL, &(data_ptr->phrase_location.w), &(data_ptr->phrase_location.h));


    
}

void render(SDL_Renderer *renderer, AppData *data_ptr)
{
    // erase renderer content
    SDL_SetRenderDrawColor(renderer, 235, 235, 235, 255);
    SDL_RenderClear(renderer);
    
    // TODO: draw!
    SDL_RenderCopy(renderer, data_ptr->penguin, NULL, &(data_ptr->penguin_location));

    SDL_RenderCopy(renderer, data_ptr->phrase, NULL, &(data_ptr->phrase_location));


    // show rendered frame
    SDL_RenderPresent(renderer);
}

void listDirectory(std::string dirname, char ***result)
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
    std::sort(files.begin(), files.end(), compareNoCase);



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
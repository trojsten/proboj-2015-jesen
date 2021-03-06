#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <unistd.h>
#include <string>
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
using namespace std;

#include "logic.h"

#define FPS 30
#define TIMEOUT 5.0

#define RYCHLOST_KAMERY 1000
#define RYCHLOST_ZOOMU  2.0
#define MIN_ZOOM        0.01
#define MAX_ZOOM        7.0

#define FARBA_POZADIA   0x202020

int main(int argc, char *argv[]) {
  if (argc != 2 || argv[1][0] == '-') {
    fprintf(stderr, "usage: %s <zaznamovy-adresar>\n", argv[0]);
    return 0;
  }

  srand(time(NULL) * getpid());

  string zaznamovyAdresar(argv[1]);
  Hra hra;
  hra.nacitajSubor(zaznamovyAdresar + "/observation");

  SDL_Init(SDL_INIT_VIDEO);
  atexit(SDL_Quit);

  TTF_Init();
  atexit(TTF_Quit);

  IMG_Init(IMG_INIT_PNG);
  atexit(IMG_Quit);

  SDL_Surface *screen = SDL_SetVideoMode(1200, 800, 32, SDL_SWSURFACE);

  string title = string("Observer - ") + argv[1];
  SDL_WM_SetCaption(title.c_str(), title.c_str());

  TTF_Font *font = TTF_OpenFont("observer/FreeSansBold.ttf", 16);

  Obrazky obrazky;
  obrazky.nacitaj("asteroid", "observer/obrazky/asteroid.png");
  obrazky.nacitaj("earth", "observer/obrazky/earth.png");
  obrazky.nacitaj("troll", "observer/obrazky/trollface.png");

  Kamera kamera(Bod(hra.sirka / 2, hra.vyska / 2), min(0.7 * screen->w / hra.sirka, 0.7 * screen->h / hra.vyska));
  Bod pohybKamery(0, 0);
  int zoomKamery = 0;

  double cas = 0.0;   // na ktorom ticku hry sme, samozrejme az po zaokruhleni
  double rychlost = 1.0;
  double timeout = 0.0;
  bool pauza = false;

  while (true) {
    int pocetTickovNaZaciatku = SDL_GetTicks();

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) return 1;

      if (event.type == SDL_KEYDOWN) {
        int key = event.key.keysym.sym;

        switch (key) {
          case SDLK_RETURN: {
            cas = 0;
          } break;

          case SDLK_p: {
            pauza = !pauza;
          } break;

          case SDLK_r: {
            rychlost *= -1;
          } break;

          case SDLK_EQUALS:
          case SDLK_PLUS: {
            rychlost *= 1.25;
          } break;

          case SDLK_MINUS: {
            rychlost *= 0.8;
          } break;

          case SDLK_LEFT: {
            pohybKamery.x -= RYCHLOST_KAMERY;
          } break;

          case SDLK_RIGHT: {
            pohybKamery.x += RYCHLOST_KAMERY;
          } break;

          case SDLK_UP: {
            pohybKamery.y -= RYCHLOST_KAMERY;
          } break;

          case SDLK_DOWN: {
            pohybKamery.y += RYCHLOST_KAMERY;
          } break;

          case SDLK_1: {
            zoomKamery -= 1;
          } break;

          case SDLK_2: {
            zoomKamery += 1;
          } break;
        }
      } else if (event.type == SDL_KEYUP) {
        int key = event.key.keysym.sym;

        switch (key) {
          case SDLK_LEFT: {
            pohybKamery.x += RYCHLOST_KAMERY;
          } break;

          case SDLK_RIGHT: {
            pohybKamery.x -= RYCHLOST_KAMERY;
          } break;

          case SDLK_UP: {
            pohybKamery.y += RYCHLOST_KAMERY;
          } break;

          case SDLK_DOWN: {
            pohybKamery.y -= RYCHLOST_KAMERY;
          } break;

          case SDLK_1: {
            zoomKamery += 1;
          } break;

          case SDLK_2: {
            zoomKamery -= 1;
          } break;
        }
      }
    }

    kamera.pozicia = kamera.pozicia + pohybKamery * (1.0 / kamera.zoom) * (1.0 / FPS);
    kamera.zoom *= 1.0 + RYCHLOST_ZOOMU * zoomKamery * (1.0 / FPS);
    if (kamera.zoom < MIN_ZOOM) kamera.zoom = MIN_ZOOM;
    if (kamera.zoom > MAX_ZOOM) kamera.zoom = MAX_ZOOM;

    if (!pauza) cas += rychlost;
    if (cas < 0) cas = 0;
    if (cas >= hra.framy.size()) {
      cas = hra.framy.size() - 1;
      timeout += 1.0 / FPS;
    } else {
      timeout = 0.0;
    }
    if (timeout >= TIMEOUT) {
      return 0;
    }

    int tick = floor(cas);

    SDL_FillRect(screen, &screen->clip_rect, FARBA_POZADIA);    // vycistime obrazovku

    hra.framy[tick].kresli(screen, kamera, obrazky, font);      // nakreslime sucasny stav

    SDL_Flip(screen);                                           // zobrazime, co sme nakreslili, na obrazovku

    int pocetTickovNaKonci = SDL_GetTicks();
    SDL_Delay(max(1, 1000 / FPS - (pocetTickovNaKonci - pocetTickovNaZaciatku)));
  }
}

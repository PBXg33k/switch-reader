# E-Hentai Switch Reader
## Usage
SSL support isn't in switch-curl - For the time being, you must run a python3 webserver as a relay and configure it in config.h (See web.py)
## Build Dependencies
Latest libnx, switch-curl, switch-libxml2, switch-(sdl sdl_image sdl_ttf), switch-json-c
available from dkp-pacman.
## Feature Plan
- Search
  - Downloaded Galleries
	- By tag 
	- By keyword
	- By rating 
	- By language (saved)
- Gallery Browser
  - Downloaded Galleries
  - Thumbnail load threading
  - Scrolling
  - Tag List
  - User Favourites
  - ExHentai Login
- Gallery View
  - Full image load threading
  - Rotation
  - Page select by Numpad
  - Skip to start/end
  - Favourite
  - Download

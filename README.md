# Hentai Switch Reader

## Supports E-Hentai, Exhentai (+ user accounts) and Nhentai domains

## Usage

### Proxy Server

This application requires a proxy server to access HTTPS website. (Until SSL issues are addressed)

Web.py is provided in tools and requires Python 3 with the fake-useragent module to run.

By default this is run on port 5000, but you can modify that yourself in web.py and in the settings menu of the application.

**Warning**

This is sending data over HTTP. It is not secure. Your username and password are sent this way to log in, be aware of this if you choose to do so. Your plain text username and password are not stored by the proxy server or the application. Your login cookies will be stored by the application to prevent the need to log in each time you load it. 

### Running

Place switch-reader.nro in your /switch/ folder of your SD card and run through the Homebrew Launcher.

A config file is generated in /switch/Reader/ and cookies are also stored in this folder. 

Please see **Configuration** below to set up the **Proxy** setting before reading the rest below.

The Red Box in the top right will return to a previous screen, or exit the application (If on the Browser screen). This is used in every screen.

A default Non-H search will load. Drag left and right to scroll through the results. Press on one to select it. 

Press **Load Gallery** to load a preview of it with tags listed. Pressing the Red Box will return to the Browser screen.

Pressing **Load Gallery** again will load it for viewing. Press the right or left of the screen to move forwards or backwards through the pages. Pressing **X** will rotate to portrait mode. Pinch to zoom, drag to move. Pressing the Red Box will return you to the Browser screen.

**Search** - Searches for galleries. Press on a category in the top right to toggle it on or off. If they are all off or all on, it will not filter by category. Pressing the Text Box here will open a keyboard for you to enter your search phrase. Pressing the red box in the top right will return to the Search screen, again another time will complete the search and load results on the Browser screen.

**Favourites** - Lists your favourited galleries. If you aren't logged in this will be empty. Use settings to log in.

**Settings** - Settings are all described in **Configuration**. The config file is saved on exit of this screen.

### Configuration

Press 'Settings' on the Browser screen. (first on loading)

**Proxy** - Set this to the full proxy URL. This will be in the form http://<ip_address>:5000/?url= unless you've modified the port number.

**Theme** - Light and Dark themes are provided, if you want a specific theme, raise an issue.

**Domain** - Toggles the domain to be searched, make sure your account has Exhentai priveleges before using Exhentai, otherwise it won't load anything.

Note - Favourites show Exhentai galleries regardless. Loading them will present failed loading images if E-hentai is set.

**Add/Del Szuru** - Add custom Szurubooru domains (See https://github.com/rr-/szurubooru). You will be asked for name first, then a domain URL. The URL must be the root of the website with a trailing slash - E.G https://example.com/szuru/ if the api is available at https://example.com/szuru/api , logins are required for now (Guest support planned)

**Username / Password** - Enter your username and password in here to login. Login will begin once you exit the Settings screen. Check the bottom left of the Browser screen, if your username is visible it was successful. This is wiped after exiting the Settings screen. **Cookies and/or auth tokens are kept locally**

## Building

Run 'make' from root directory. Will output to switch-reader.nro

## Build Dependencies
Latest libnx, switch-curl, switch-libxml2, switch-(sdl sdl_image sdl_ttf), switch-json-c
available from dkp-pacman.

## Current Planned Features
- Domains
  - Various Booru site support
- Search
  - Filters
 	- History of Searches (As an option)
	- Permanent filters
- Gallery Preview
  - Comments
  - Related galleries (If domain supports it)
- Gallery
  - Custom local galleries
  - Page select by Numpad
  - Skip to start/end
  - (Un)Favourite gallery

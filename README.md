# cairo-sdl2-software-rendering
A test project to check the performance and memory usage of software rendering in SDL2 with help of Cairo.

![Screenshot](screenshots/RocketRacoon.png)

### Note on usgae of Icon Fonts
Here I have used [JetBrains Nerd Font](assets/fonts) for icons, if you have your own icon font, you should be converting the icon codepoint to utf-8 encoded string, then render the icon with `cairo_show_text()`. The function to convert codepoint to utf-8 encoded string is [here](https://github.com/Eessh/cairo-sdl2-software-rendering/blob/090b5b881cfaab88c935365b4c422b2be1ab3f7b/main.c#L61).

### Building
- Windows:
  ```powershell
  .\scripts\windows\gen_makefiles.bat
  .\scripts\windows\build_and_run_release.bat
  ```
- Linux:
  🚧 WIP
- macOS:
  🚧 WIP

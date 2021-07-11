![atlas](https://user-images.githubusercontent.com/23220511/123022435-6acacf80-d39b-11eb-9aa8-7710ecb5b4f9.png)

# atlas
Command line texture atlas generation tool

# Formats
* PNG
* I will be adding support for more formats as I need them

# Depenencies (included)
* stb_rect_pack (Sean Barrett)
* stb_image & stb_image_write (Sean Barrett)
* cute_files

# Usage
* Add atlas.exe to the system PATH
* Command line: `atlas [ format ]` compiles all images of the specified format located in the working directory into a single image

# Compiling (build.bat)
* The source includes a build.bat file which will build the executable. No other work required!

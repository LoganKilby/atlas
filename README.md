![atlas](https://user-images.githubusercontent.com/23220511/123022435-6acacf80-d39b-11eb-9aa8-7710ecb5b4f9.png)

# atlas
Texture atlas generation tool written in C for the .png file format

# Formats
* PNG
* All I have needed so far personally are png files, but I will be adding support for more as I need them

# Depenencies (included)
* stb_rect_pack (Sean Barrett)
* stb_image & stb_image_write
* cute_files

# Usage
* Add atlas.exe to the system PATH
* Command line: `atlas [ format ]` compiles all images of the specified format located in the working directory into a single image

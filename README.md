# SoftRenderer
C++ 3D software rendering engine written for personal didactic purposes and to get access to an open environment ready for quick test and debug of new rendering techniques. 
The image shows a gun rendered with the engine (the animated video is available on [YouTube](https://youtu.be/o1CB-Vv9PXo))
![SoftRenderer demo](https://raw.githubusercontent.com/AndreaLu/SoftRenderer/main/screenshot.png)
The demo is built using freely available public access assets:
* Monvalley Dirtroad environment map from [sIBL Archive](http://www.hdrlabs.com/sibl/archive.html)
* Cerberus 3D PBR Gun from [Alexander Maximov](https://artisaverb.info/Cerberus.html)
## Required libraries 
* [OpenGL Mathematics](https://glm.g-truc.net/0.9.9/index.html): fast and convenient implementation of linear algebra methematical operations

The project also uses the handy single-file header libraries from [STB](https://github.com/nothings/stb) for picture files loading and writing - the files ([stb_image.h](https://github.com/AndreaLu/SoftRenderer/blob/main/stb_image.h), [stb_image_write.h](https://github.com/AndreaLu/SoftRenderer/blob/main/stb_image_write.h)) are already included 

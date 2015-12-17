# CS1566-Final

Final project design for CS1566 Introduction to Computer Graphics. Divided into a helper application that can "record" files by nagivating through a scene (mfg.cc) and one that takes the generated file plus a scene and renders each frame using a raytracer (moviemaker.cc).    
    
mfg.cc    
-takes a scene file which specifies objects and their positions in a scene from the command line    
-user can move through the scene and press 'r' to start and stop recording, which writes data to an auxilary file used by moviemaker.cc    
    
moviemaker.cc    
-takes a generated .mcf file as well as a the associated scene file, and renders then saves each frame as a .tga image with the name "frame0" and increasing by 1 each frame    
    
kirby.txt    
-example of a scene file specification

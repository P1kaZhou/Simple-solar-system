# TP openGL - ZHOU Daniel

In this report, I will explain what I did, how I did it, and what I learn from every big part of the practical work. 

# 1) Codebase
In this part, I just checked the code and a little of the resources. The commands where the biggest thing I learned, as I automaticallly use the 3 commands everytime I want to see the progress

# 2) A Single Triangle
In this part, I roughly learned what every piece of hardware was doing with the things implemented. It was also my first contact with all the very important functions in opengl, such as `glEnableVertexAttribArray` (to cite one among many others).

My first problem was that I couldn't load the colors. But this was because I didn't name correctly the variables in my shaders. The `out` in `vertexShader.glsl` and `in` in `fragmentShader.glsl` should have the same names, got it!

# 3) Sphere mesh

By using the spherical coordinates as well as by converting them into cartesian ones, it's quite easy to get an array of all vertex positions. <br>
However there were always a problem with dealing with a shared pointer. So instead I created the `Mesh` class which was outside `main`. I declared the meshes in `main` and in `main`, genSphere was basically `initCPUGeometry()` by initialiazing the sphere with creating the vectors containing all the needed information and according to the requirements (size, starting position etc)
As I needed to render several spheres, the code in main (which could be usable for only one shape) was moved into mesh, and each instance of mesh could call the different functions like `initGPUGeometry()` of `render()`. And in the same functions in main were just each mesh calling these functions. <br>
This somehow reminded me that yes, in Object Oriented programming, it's good to deleguate the functions and the work to do. <br>
There really weren't any issues with the shading, all I had to do is to translate the equations into code.

# 4) The three planets

As I already split the code, there weren't any issues for the colors. Except that I couldn't really set the colors at first, which was resolved by taking a peek at what `glGetUniformLocation` does.

# 5) Animation

This step was special. I knew what to do, but didn't know how to do it. In this state, I did the easiest first, which was to compute the next positions when knowing the current time. <br>
Next step was to look into some tutorials, and splitting the work once again. Translating first, then scaling, and finally rotating. <br>
Of course, I added these attributes to `Mesh` and set these when initializing the gpu programs. <br>
I checked learnopengl and knew what to do at each render step, and added the necessary code in the shaders as well. By checking other websites I also knew the order, and that the translation matrix had to be multiplied by the scaling matrix

# 6) The textures

This case was interesting, because what blocked me what a stupid problem, the fact that `m_textureCoords` was empty. And that was because of a casting problem :(

But I got to know many things, related to how to load texture from the perspective of the user, but also what the hardware do with the texture, what deals with it and when. 

But I only saw that there were a casting problem at the very end. Before, I tried to look at many different things, like array binding, etc...
For instance, once I got a segfault, and that was because of wrong memory access (obviously), but the most interesting thing in that segfault was that I learnt what memory is allowed for the different kind of data. <br>
I discovered the casting problem when I tried to hardcode the `m_textureCoords` by putting predefined values in it. But it still didn't work. The casting was above these commands, and swapping these two finally worked!

~~Well, maybe it's better to check twice before copypasting some code~~

# 7) More...?

If you look at the code, you might see a `mercuryMesh`. This is because I tried to add mercury in the solar system, but somehow some textures were swapped. Well... looks like I will always have something to learn in openGL!
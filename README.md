# moyai

An experimental simple 2D game library for native C++ and OpenGL with cloud gaming support,
strongly inspired by MoaiSDK API structure.

Moai SDK: https://github.com/moai/moai-dev

# Features
 - Rendering
  - 2D sprite + 3D mesh
  - OpenGL 2.0 (ES1), [GLFW3 http://www.glfw.org/]
 - Image
  - PNG read/write from file (lodepng)
  - dynamic texture
 - Audio
  - Reads any WAV files
  - FMOD based ( http://fmod.com/ )
 - Font
  - FreeType2 based, reads TTF files, multi-byte chars
 - Headless mode for cloud gaming
  - Transfer all geometry data every frame via TCP, to enable remote rendering.
  - Recording all sprite stream and save it in file, replaying the file.

# Compatibility
 - OSX 10.10/10.11
 - Windows 7,8.1,10 (VS2012,2013,2015)
 - working on iOS 9 or later
 - Linux (For cloud gaming server)
 
# Performance target
10K~20K sprites at 60FPS with -O0 on OSX/Linux, -O1 on VC++


# Example

All of its class names are from MoaiSDK's, 
and the order of API calls is also completely same as MoaiSDK's.

Some arguments are modified to achieve even simpler usage.



## Sprite basics

To show a sprite on your screen, just:

<pre lang="c++">
<code>
#include "moyai/client.h"

Moyai *g_moyai;

int main() 
{
  // initialize glfw
  glfwInit();
  GLFWwindow *w = glfwCreateWindow(1024,768, "demo", NULL, NULL );
  g_moyai = new MoyaiClient(w);

  Viewport *v = new Viewport();
  v->setSize(1024,768);
  v->setScale(1024,768);

  Layer *lay = new Layer();
  lay->setViewport(v);
  
  Camera *c = new Camera();
  c->setLoc(0,0);
  lay->setCamera©;

  g_moyai->insertLayer(lay);

  Texture *tex = new Texture();
  tex->load("spritesheet.png");

  TileDeck *deck = new TileDeck();
  deck->setTexture(tex);
  deck->setSize( 16,16, 16,16); //16x16 of 16pix x 16pix sprites in a sheet

  Prop2D *p = new Prop2D();
  p->setDeck(deck);
  p->setLoc(100,100);
  p->setIndex(13); // index number in the sprite sheet (top-down, left-right)
  p->setScl(32,32);

  lay->insertProp(p);



  while(1) {
    static double last_poll_at = 0;
    double t = now();
    double dt = t - last_poll_at;
  
    g_moyai->poll(dt); // everything happens in 
    g_moyai->renderAll();
    
    last_poll_at = t;
  }
}

</code>
</pre>

## 3D basics
<pre lang="c++">
<code>
int main() {
  glfwInit();
  GLFWwindow *w = glfwCreateWindow( SCRW,SCRH, "demo", NULL, NULL );

  g_moyai = new MoyaiClient(w);
  glfwSetWindowTitle( "demo3d");
  glfwEnable( GLFW_STICKY_KEYS );
  glfwSwapInterval(1); // vsync

  glClearColor(0,0,0,1);
  glEnable(GL_DEPTH_TEST);    
  glEnable(GL_DEPTH_BUFFER_BIT);    
  glDepthMask(true );
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
    
  Viewport *v = new Viewport();
  v->setSize(1024,768);
  v->setClip3D( 0.01, 100 ); // near and far clip
  
  Camera *cam = new Camera();
  cam->setLoc( Vec3(0,0,10) );
  
  Layer *layer = new Layer();
  layer->setCamera(cam);
  layer->setViewport(v);

  Texture *t = new Texture();
  t->load("wood_surface.png");
  Prop3D *p = new Prop3D();
  p->setMesh(mesh); // prepare mesh before
  p->setTexture(t);
  p->setScl(Vec3(2,2,2));
  p->setLoc(Vec3(1,0,0));
  layer->insertProp(p);

  while(1){
    static double last_poll_at = now();
    double t = now();
    double dt = t - last_poll_at;
    g_moyai->poll(dt);
    g_moyai->renderAll();
    last_poll_at = t;
  }
  
}
</code>
</pre>

## Audio

To use sound effects or background music, just use SoundSystem and Sound.

It is simply based on FMOD: stable, small and lightweight.

<pre lang="c">
<code>
SoundSystem *ss = new SoundSystem();
Sound *s = ss->newSound( "explode.wav" );
s->play();
</code>
</pre>

Fonts
----
To use TrueType fonts, just use Font and TextBox.
Pass wchar_t string to prepare glyphs in texture buffer.
Moyai only supports pre-loaded fonts.


<pre lang="c">
<code>
wchar_t charcodes[] = L" !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~あいうえおぁぃぅぇぉかきくけこがぎぐげごさしすせそざじずぜぞたちつてとだぢづでどなにぬねのはひふへほばびぶべぼぱぴぷぺぽまみむめもやゆよゃゅょらりるれろわをん";    
 
Font *f = new Font();
f->loadFromTTF( "cinecaption227.ttf", charcodes, 12 ); 

TextBox *tb = new TextBox();
tb->setFont(f);
tb->setString("こんにちはABC");

layer->insertProp(tb);

</code>
</pre>

Screen capture
----
<pre lang="c">
<code>
Image *img = new Image();
img->setSize(1024,768);
g_moyai->capture(img);
img->writePNG("captured.png");
delete img;
</code>
</pre>

Headless
----
Simply create RemoteHead and startServer, then setup necessary things.

<pre lang="c">
<code>
Moyai::globalInitNetwork();

RemoteHead *rh = new RemoteHead();
rh->startServer(22222);

moyai_client->setRemoteHead(rh);
rh->setTargetMoyaiClient(moyai_client);

rh->enableSpriteStream(); // If you want to use sprite stream
g_rh->enableVideoStream(SCRW*RETINA,SCRH*RETINA,3); // If you want to use video stream with 4 times less resolution

        
sound_system->setRemoteHead(rh); // If you want to transfer sounds
rh->setTargetSoundSystem(sound_system);

keyboard->setRemoteHead(rh); // If you want to receive remote keyboard input
rh->setTargetKeyboard(keyboard);

mouse->setRemoteHead(rh); // If you want to receive remote mouse input
rh->setTargetMouse(mouse);
</code>
</pre>

Use viewer program (Implemented in vw.cpp) as a head of the headless server.

Command line:

<code>./viewer localhost</code>

This will connect to the server and render the sprite stream.
Sprite stream is a binary data stream of sprites in Moyai game application program
that includes all sprites' position, sprite-atlas index, scale, rotation, shader, color,
and any necessary data for rendering.


Video Streaming
=====
You can try video streaming by adding <code>--videostream</code> to demo2d program.
By adding this option, you will get both sprite stream and video stream at the same time,
so you can omit sprite stream by adding <code>--skip-spritestream</code> option to demo2d.

Please see these command line examples:
<code>
<pre>
bash$ ./demo2d --headless    # This only sends sprite stream
bash$ ./demo2d --headless --videostream # This sends both sprite stream and video stream
bash$ ./demo2d --headless --videostream --skip-spritestream   # This sends only video stream
</pre>
</code>

    
Build
=====

MacOS X 10.8,10.9,10.10,10.11: After installing Xcode and command line tools, just do:
 
<pre>
$ brew install libuv # for uv.h, if necessary.
$ git clone https://github.com/kengonakajima/moyai
$ cd moyai
$ make
$ ./demo2d
$ ./demo3d
$ ./dyncam2d
</pre>

Windows 7/8/8.1 VisualStudio 2012/2013 :

1. git clone https://github.com/kengonakajima/moyai   or use Github For Windows from github.com
2. Start from demowin/demowin.sln
3. Build a project "demowin" 
4. Run(Debug) the program 
</pre>

Demo programs
=====
demo2d, demo3d, dyncam2d are demo programs.
demo2d.cpp for <code>demo2d</code>, demo3d.cpp for <code>demo3d</code>, dyncam2d.cpp for <code>dyncam2d</code> .

<code>demo2d</code> demonstrates basic 2D features.

<img src="https://raw.github.com/kengonakajima/moyai/master/screen_shot_2d.png" width=800>

By adding "--headless" option, it runs as a RemoteHead server, so you can view a sprite stream by <code>viewer</code>.

Command line:

<pre>
bash$ ./demo2d --headless
bash$ ./viewer
</pre>

<code>demo3d</code> demonstrates basic 3D features, but it is too early to use.

<img src="https://raw.github.com/kengonakajima/moyai/master/screen_shot_3d.png" width=800>

<code>dyncam2d</code> demonstrates how to use 2D dynamic cameras by implementing simple top view
field and player characters walking on it.

Command line:

<pre>
bash$ ./dyncam2d
</pre>

<img src="https://raw.github.com/kengonakajima/moyai/master/dyncam2d_ss.png" width=800>

dyncam2d runs always as a RemoteHead server, so you'll use <code>viewer</code> to view sprite stream.

Command line:

<pre>
bash$ ./dyncam2d   # field shows up, but no players in screen
bash$ ./viewer &   # First player character shows up
bash$ ./viewer &   # Second player character shows up
bash$ ./viewer &   # and so on..
</pre>

Using replay server
=====
When you use headless sprite stream, the sprite stream is saved at "/tmp/moyaistream_1_1461123550_487924" in OSX.
You can see the saved sprite stream by using replay server.
A command "replayer" reads a sprite stream file and listen to a TCP port and send the stream to the viewer client.

<pre>
bash$ ./replayer /tmp/moyaistream_1_1461123550_487924 --once
bash$ ./viewer localhost
</pre>

Classes
=====
List of classes defined in moyai.h :

- Basic types
  - <code>Vec2</code>
  - <code>Color</code>
  - <code>Moyai</code>
  - <code>Layer</code>
  - <code>Viewport</code>
  - <code>Camera</code> 
  - <code>Prop2D</code> : simple sprite
  - <code>Prop3D</code> 
  
- Rendering and Graphics
  - <code>Image</code>  : image in memory
  - <code>Texture</code> : opengl texture
  - <code>TileDeck</code> : sprite sheet
  - <code>ColorReplacerShader</code> : replace pixel color of given texture 
  - <code>Grid</code> : m x n grid
  - <code>CharGrid</code> : for simple ascii bitmap fonts
  - <code>Prim</code> : draw lines and rectangles
- Text and fonts  
  - <code>Font</code> : truetype font
  - <code>TextBox</code>
 
- Animation
  - <code>AnimCurve</code>
  - <code>Animation</code>  

- Audio
  - <code>SoundSystem</code>
  - <code>Sound</code>

- Headless
  - <code>RemoteHead</code>
  - <code>Client</code>


TODO
=====
 - Correct serialization required for structures sent on sprite stream.
 - Browser/mobile version of sprite stream viewer

License notes
=====
See LICNSE.txt for Moyai itself.

 - Graphics resources are from Oryx Design Lab: http://oryxdesignlab.com/
 - Music from: Erik Satie Gymnopedie no.1
 - lodepng image loader : Copyright (c) 2005-2013 Lode Vandevenne
 - zlib :  (C) 1995-2012 Jean-loup Gailly and Mark Adler
 - libuv : Part of Node project.
 - bzip2 : copyright (C) 1996-2010 Julian R Seward
 - FMOD : FMOD NON-COMMERCIAL LICENSE
 - FreeType : Copyright 1996-2002, 2006 by David Turner, Robert Wilhelm, and Werner Lemberg 
 - freetype-gl : Copyright 2011,2012 Nicolas P. Rougier. All rights reserved.
 - GLFW Copyright (c) 2002-2006 Marcus Geelnard, 2006-2010 Camilla Berglund <elmindreda@elmindreda.org>
 - lz4 : 2013 Yann Collet 
 - UNTZ : Copyright 2010-2011 Zipline games, CPAL license
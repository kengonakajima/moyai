# moyai

An experimental simple 2D game engine for native C++, strongly inspired by MoaiSDK API structure.

Moai SDK: https://github.com/moai/moai-dev

 - Rendering
  - OpenGL 2.0 (ES2), GLUT
 - Image
  - PNG read/write from file
  - dynamic texture
 - Audio
  - Reads any WAV files
  - FMOD based ( http://fmod.com/ )
 - Font
  - FreeType2 based, reads TTF files, multi-byte chars
  

# Small
Moyai is just combining good existing stable libs so itself has only 2K lines of C++ code.

<pre>
[@Macintosh-2 moyai]$ wc moyai.cpp cumino.cpp moyai.h
     866    2770   26227 moyai.cpp
     196     525    4264 cumino.cpp
     914    2965   24696 moyai.h
    1976    6260   55187 total
</pre>

# Example

All of its class names are from MoaiSDK's, 
and the order of API calls is also completely same as MoaiSDK's.

Some arguments are modified to achieve even simpler usage.



## Sprite basics

To show a sprite on your screen, just:

<pre lang="c++">
<code>
Moyai *g_moyai;

int main() 
{
  g_moyai = new Moyai();

  Viewport *v = new Viewport();
  v->setSize(1024,768);
  v->setScale(1024,768);

  Layer *l = new Layer();
  lay->setViewport(v);
  
  Camera *c = new Camera();
  c->setLoc(0,0);
  lay->setCamera©;

  g_moyai->insertLayer(lay);

  Texture *tex = new Texture();
  tex->load("spritesheet.png");

  TileDeck *deck = new TileDeck();
  deck->setTexture(tex);
  deck->setSize( 16,16, 16,16, 256,256); //16x16 of 16pix x 16pix sprites in a sheet

  Prop *p = new Prop();
  p->setDeck(deck);
  p->setLoc(100,100);
  p->setIndex(13); // index number in the sprite sheet (top-down, left-right)
  p->setScl(32,32);

  lay->insertProp℗;

  // initialize glut
  glutInit();
  …
  glutIdleFunc(idle_callback);        
  glutDisplayFunc(display_callback);
  
  glutMainLoop();
}

void idle_callback() 
{ 
  static double last_poll_at = 0;
  double t = now();
  double dt = t - last_poll_at;
  
  g_moyai->pollAll(dt); // everything happens in 
  
  last_poll_at = t;

  display();
}
void display_callback() 
{
  g_moyai->renderAll();
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

Build
=====

Tested only on MacOS X 10.8. After installing Xcode and command line tools, just do:
 
<pre>
$ git clone https://github.com/kengonakajima/moyai
$ cd moyai
$ make
$ ./demogame
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
  - <code>Prop</code> : a sprite 
  
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

- Audio
  - <code>SoundSystem</code>
  - <code>Sound</code>
 
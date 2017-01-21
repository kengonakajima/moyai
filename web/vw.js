
var g_ws;

var g_moyai_client;

//var g_keyboard = new Keyboard();
//var g_mouse = new Mouse();
//var g_pad = new Pad();



function onPacket(ws,pkttype,argdata) {
    switch(pkttype) {
    case PACKETTYPE_ZIPPED_RECORDS:
        {
            //                console.log("zipped records:",argdata);
            var uncompressed = Snappy.uncompress(argdata.buffer);
            console.log(uncompressed);
            var dv = new DataView(uncompressed);
            ws.unzipped_rb.push(dv,uncompressed.byteLength);
            console.log("unzipped_rb:",ws.unzipped_rb);
        }
        break;
    case PACKETTYPE_S2C_WINDOW_SIZE:
        {
            var dv = new DataView(argdata.buffer);
            var w = dv.getUint32(0);
            var h = dv.getUint32(4);
            console.log("received window_size:",w,h);
        }
        break;
    }
}




// button funcs
function connectButton() {
    g_ws = createWSClient("ws://localhost:8888/");
    g_ws.onPacket = onPacket;
}
function disconnectButton() {
    g_ws.close();
}


// loop
function animate() {
	requestAnimationFrame( animate );
    if(g_moyai_client) {
        g_moyai_client.render();
    }
}

    
animate();


/////////// testing

g_moyai_client = new MoyaiClient(400,400,window.devicePixelRatio);
document.getElementById("screen").appendChild( g_moyai_client.renderer.domElement );

var g_viewport = new Viewport();
g_viewport.setSize(400,400);
g_viewport.setScale2D(400,400);

var g_main_layer = new Layer();
g_moyai_client.insertLayer(g_main_layer);
g_main_layer.setViewport(g_viewport);

var g_hud_layer = new Layer();
g_moyai_client.insertLayer(g_hud_layer);
g_hud_layer.setViewport(g_viewport);

var g_camera = new Camera();
g_camera.setLoc(0,0);
g_main_layer.setCamera(g_camera);

var g_filedepo = new FileDepo();
g_filedepo.ensure("base.png",base_png);
g_filedepo.ensure("blobloblll.wav", blobloblll_wav );
//g_filedepo.ensure("cinecaption227.ttf", cinecaption227_ttf );
g_filedepo.ensure("dragon8.png", dragon8_png );
g_filedepo.ensure("font_only.png", font_only_png );
//g_filedepo.ensure("gymno1short.wav", gymno1short_wav );



console.log("PNG",PNG);

var g_base_atlas = new Texture();
g_base_atlas.loadPNGMem( base_png );
g_base_deck = new TileDeck();
g_base_deck.setTexture(g_base_atlas);
g_base_deck.setSize(32,32,8,8 );

    
/*

    // atlas
    g_base_atlas = new Texture();
    g_base_atlas->load("./assets/base.png");
    g_base_deck = new TileDeck();
    g_base_deck->setTexture(g_base_atlas);
    g_base_deck->setSize(32,32,8,8 );

    g_bmpfont_atlas = new Texture();
    g_bmpfont_atlas->load("./assets/font_only.png");
    g_bmpfont_deck = new TileDeck();
    g_bmpfont_deck->setTexture( g_bmpfont_atlas );
    g_bmpfont_deck->setSize(32,32, 8,8 );

    //    
    Layer *tmplayer = new Layer();
    g_moyai_client->insertLayer(tmplayer);
    tmplayer->setViewport(g_viewport);

    Texture *t = new Texture();
    t->load( "./assets/base.png" );

    Texture *t2 = new Texture();
    t2->load( "./assets/base.png" );
    
    TileDeck *deck = new TileDeck();
    deck->setTexture(t);
    deck->setSize(32,32,8,8);
    TileDeck *d2 = new TileDeck();
    d2->setTexture(t2);
    d2->setSize(32,32,8,8 );

    Prop2D *sclp = new Prop2D();
    sclp->setDeck(deck);
    sclp->setIndex(1);
    sclp->setScl(16,16);
    sclp->setLoc(-200,0);
    sclp->seekScl( 128,128, 8);
    sclp->setRot( M_PI/8 );
    g_main_layer->insertProp(sclp);

    Prop2D *sclprot = new Prop2D();
    sclprot->setDeck(deck);
    sclprot->setIndex(0);
    sclprot->setScl(16,16);
    sclprot->setLoc(-300,0);
    sclprot->seekScl( 128,128, 8);
    sclprot->setRot( M_PI/8 );
    sclprot->seekRot( M_PI*2, 4 );
    sclprot->setUVRot(true);
    g_main_layer->insertProp(sclprot);    

    Prop2D *colp = new Prop2D();
    colp->setColor(1,0,0,1);
    colp->setDeck(d2);
    colp->setIndex(1);
    colp->setScl(24,24);
    colp->setLoc( range(-100,100), range(-100,100));
    g_main_layer->insertProp(colp);

    Prop2D *statprimp = new Prop2D(); // a prop that has a prim with no changes
    statprimp->setDeck(g_base_deck);
    statprimp->setIndex(1);
    statprimp->setColor(0,0,1,1);
    statprimp->addLine(Vec2(0,0),Vec2(1,1),Color(1,1,1,1), 3);
    statprimp->setLoc(100,-100);
    g_main_layer->insertProp(statprimp);

    // static grids
    {
        Prop2D *p = new Prop2D();
        p->setDeck(d2);
        p->setScl(24,24);
        p->setLoc(-100,-300);
        Grid *g = new Grid(8,8);
        g->setDeck(d2);
        int iii=1;
        for(int x=0;x<8;x++){
            for(int y=0;y<8;y++){
                g->set(x,y,iii % 3);
                g->setColor( x,y, Color(range(0.5,1), range(0.5,1), range(0.5,1), range(0.5,1) ));
                if(x==0) g->setXFlip(x,y,true);
                if(x==1) g->setYFlip(x,y,true);
                if(x==2) g->setUVRot(x,y,true);
                if(x==3) {
                    Vec2 ofs(0.5,0.5);
                    g->setTexOffset(x,y,&ofs);
                }
                iii++;
            }
        }
        p->addGrid(g);
        g_main_layer->insertProp(p);

        Prop2D *p2 = new Prop2D();
        p2->setColor(1,1,0,0.5);
        p2->setDeck(d2);
        p2->setScl(12,12);
        p2->setLoc(-100,100);
        p2->addGrid(g);
        tmplayer->insertProp(p2);
    }
    

    

    int bulletinds[] = { ATLAS_BULLET0, ATLAS_BULLET0+1, ATLAS_BULLET0+2,ATLAS_BULLET0+3};
    g_bullet_anim_curve = new AnimCurve( 0.2, true, bulletinds, elementof(bulletinds));
    int digitinds[] = { ATLAS_DIGIT0, ATLAS_DIGIT0+1, ATLAS_DIGIT0+2, ATLAS_DIGIT0+3 };
    g_digit_anim_curve = new AnimCurve( 0.2, false, digitinds, elementof(digitinds));
    

    Texture *dragontex = new Texture();
    dragontex->load( "./assets/dragon8.png");
    for(int j=0;j<2;j++) {
        for(int i=0;i<6;i++) {
            Prop2D *p = new Prop2D();
            p->setTexture(dragontex);
            p->setScl(32,32);
            p->setColor(1,1,1,0.3);
            p->setLoc(-SCRW/2+50 + i * 10,-SCRH/2+70 + (i%2)*10 + j*80);
            if(j==0) p->use_additive_blend = true;
            //p->setLoc(-200,-200);
            g_main_layer->insertProp(p);
        }
    }
        
    ////////////////////
    wchar_t charcodes[] = L" !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~あいうえおぁぃぅぇぉかきくけこがぎぐげごさしすせそざじずぜぞたちつてとだぢづでどなにぬねのはひふへほばびぶべぼぱぴぷぺぽまみむめもやゆよゃゅょらりるれろわをん、。アイウエオァィゥェォカキクケコガギグゲゴサシスセソザジズゼゾタチツテトダヂヅデドナニヌネノハヒフヘホバビブベボパピプペポマミムメモヤユヨャュョラリルレロワヲンーッっ　「」";    

    g_font = new Font();
    g_font->loadFromTTF("./assets/cinecaption227.ttf", charcodes, 12 );


    g_tb = new TextBox();
    g_tb->setFont(g_font);
    g_tb->setString("dummy");
    g_tb->setScl(1);
    g_tb->setLoc(22,22);
    g_main_layer->insertProp(g_tb);

    TextBox *t3 = new TextBox();
    t3->setFont(g_font);
    t3->setString( L"ABC012あいうえお\nあいうえお(wchar_t)。" );
    t3->setLoc(-100,-50);
    t3->setScl(1);
    g_main_layer->insertProp(t3);


    TextBox *t4 = new TextBox();
    t4->setFont(g_font);
    t4->setString( "ABC012あいうえお\nあいうえお(mb-utf8)。" );
    t4->setLoc(-100,-90);
    t4->setScl(0.75f);
    g_main_layer->insertProp(t4);
    
    // Check bottom line
    TextBox *t5 = new TextBox();
    t5->setFont(g_font);
    t5->setString( L"THIS SHOULDN'T SINK UNDER BOTTOM LINE : このもじはしたにしずまない1ぎょうめ\n"
                   L"THIS SHOULDN'T SINK UNDER BOTTOM LINE : このもじはしたにしずまない2ぎょうめ"
                   );
    t5->setLoc(-SCRW/2,-SCRH/2);
    t5->setScl(1);
    g_main_layer->insertProp(t5);

    
    // Image manipulation
    Image *dragonimg = new Image();
    dragonimg->loadPNG( "assets/dragon8.png" );
    assert( dragonimg->width == 8 );
    assert( dragonimg->height == 8 );

    for(int y=0;y<8;y++){
        for(int x=0;x<8;x++) {
            unsigned char r,g,b,a;
            dragonimg->getPixelRaw( x,y, &r, &g, &b, &a );
            if( r == 255 && g == 255 && b == 255 && a == 255 ) {
                print("setPixelRaw: %d,%d",x,y);
                dragonimg->setPixelRaw( x,y, 255,0,0,255 );
            }
        }
    }

    Texture *dragontex0 = new Texture();
    dragontex0->setImage( dragonimg );
    
    Texture *dragontex1 = new Texture();
    dragontex1->load( "assets/dragon8.png" );

#if 1
    Prop2D *dragonp1 = new Prop2D();
    dragonp1->setLoc( SCRW/2-80, 0);
    dragonp1->setTexture(dragontex1);
    dragonp1->setScl(32);
    g_main_layer->insertProp(dragonp1);    
#endif
    
    Prop2D *dragonp0 = new Prop2D();
    dragonp0->setLoc( SCRW/2-40, 0);
    dragonp0->setTexture( dragontex0 );
    dragonp0->setScl(32);
    g_main_layer->insertProp(dragonp0);

    TileDeck *dragondk = new TileDeck();
    dragondk->setTexture(dragontex0);
    dragondk->setSize( 2,2,8,8 );
    Prop2D *dragonp2 = new Prop2D();
    dragonp2->setLoc( SCRW/2-120, 0);
    dragonp2->setDeck( dragondk );
    dragonp2->setScl(32);
    dragonp2->setIndex(1);
    g_main_layer->insertProp(dragonp2);

   
    

    // bitmap font
    Prop2D *scorep = new Prop2D();
    scorep->setLoc( -SCRW/2+32,SCRH/2-100 );
    CharGrid *scoregrid = new CharGrid(8,8);
    scoregrid->setDeck(g_bmpfont_deck );
    scoregrid->setAsciiOffset(-32);
    scoregrid->printf( 0,0, Color(1,1,1,1), "SCORE: %d",1234 );
    scoregrid->printf( 0,1, Color(1,1,0,1), "$#!?()[%s]", "hoge" );
    scoregrid->setColor( 3,0, Color(0,1,1,1));
    scorep->addGrid(scoregrid);
    g_main_layer->insertProp(scorep);

    // line prop
    g_linep = new Prop2D();
    g_narrow_line_prim = g_linep->addLine( Vec2(0,0), Vec2(100,100), Color(1,0,0,1) );
    g_linep->addLine( Vec2(0,0), Vec2(100,-50), Color(0,1,0,1), 5 );
    g_linep->addRect( Vec2(0,0), Vec2(-150,230), Color(0.2,0,1,0.5) );
    g_linep->setLoc(0,200);
    g_linep->setScl(1.0f);
    g_main_layer->insertProp(g_linep);
    // add child to line prop
    Prop2D *childp = new Prop2D();
    childp->setDeck(g_base_deck);
    childp->setScl(16,44);
    childp->seekRot( M_PI * 100, 30 );
    childp->setIndex(0);
    childp->setLoc(-222,-222);
    //g_main_layer->insertProp(childp);
    g_linep->addChild(childp);


    // dynamic images
    {
        g_img = new Image();
        g_img->setSize(256,256);
        for(int i=0;i<256;i++){
            Color c(range(0,1), range(0,1),range(0,1),1);
            g_img->setPixel( i,i, c );
        }
        g_img->writePNG( "dynamic_out.png");
        g_dyn_texture =  new Texture();
        g_dyn_texture->load("assets/base.png");
        g_dyn_texture->setImage(g_img);


    
        Prop2D *p = new Prop2D();
        TileDeck *d = new TileDeck();
        d->setTexture(g_dyn_texture);
        d->setSize( 16,16,16,16);
        p->setDeck(d);
        p->setLoc(200,200);
        p->setScl(128,128);
        p->setIndex(0);
        g_main_layer->insertProp(p);
    }
    
    g_pc = createMyShip(0,0);
    */

DIR4_NONE=-1;
DIR4_UP=0;
DIR4_RIGHT=1;
DIR4_DOWN=2;
DIR4_LEFT=3;


randomDir = function() {
    return irange(DIR4_UP,DIR4_LEFT+1);
}

reverseDir = function(d){
    switch(d){
    case DIR4_UP: return DIR4_DOWN;
    case DIR4_DOWN: return DIR4_UP;
    case DIR4_RIGHT: return DIR4_LEFT;
    case DIR4_LEFT: return DIR4_RIGHT;
    default:
        return DIR4_NONE;
    }
}

rightDir = function(d) {
    switch(d){
    case DIR4_UP: return DIR4_RIGHT; 
    case DIR4_DOWN: return DIR4_LEFT;
    case DIR4_RIGHT: return DIR4_DOWN;
    case DIR4_LEFT: return DIR4_UP;
    default:
        return DIR4_NONE;
    }
}
leftDir = function(d) {
    switch(d){
    case DIR4_UP: return DIR4_LEFT;
    case DIR4_DOWN: return DIR4_RIGHT;
    case DIR4_RIGHT: return DIR4_UP;
    case DIR4_LEFT: return DIR4_DOWN;
    default:
        return DIR4_NONE;
    }    
}

// 4方向のみ
dxdyToDir = function(dx,dy){
    if(dx>0&&dy==0){
        return DIR4_RIGHT;
    } else if(dx<0&&dy==0){
        return DIR4_LEFT;
    } else if(dy>0&&dx==0){
        return DIR4_UP;
    }else if(dy<0&&dx==0){
        return DIR4_DOWN;
    } else {
        return DIR4_NONE;
    }
}
clockDir = function(d) {
    switch(d) {
    case DIR4_NONE: return DIR4_NONE;
    case DIR4_UP: return DIR4_RIGHT;
    case DIR4_RIGHT: return DIR4_DOWN;
    case DIR4_DOWN: return DIR4_LEFT;
    case DIR4_LEFT: return DIR4_UP;
    default: console.assert( "clockDir: invalid direction:", d);
    }
    return DIR4_NONE;
}

dirToDXDY = function(d) {
    switch(d){
    case DIR4_NONE: return null;
    case DIR4_RIGHT: return {x:1,y:0};
    case DIR4_LEFT: return {x:-1,y:0};
    case DIR4_UP: return {x:0,y:1};
    case DIR4_DOWN: return {x:0,y:-1};
    default:
        console.assert("dirToDXDY: invalid direction:",d);
    }
}




////

irange = function(a,b) {
    return parseInt(range(a,b));
}
range = function(a,b) {
    var small=a,big=b;
    if(big<small) {
        var tmp = big;
        big=small;
        small=tmp;
    }
    var out=(small + (big-small)*Math.random());
    if(out==b)return a; // in very rare case, out==b
    return out;
}
sign = function(f) {
    if(f>0) return 1; else if(f<0)return -1; else return 0;
}
now = function() {
    var t = new Date().getTime();
    return t / 1000.0;
}
lengthf = function(x0,y0,x1,y1) {
    return Math.sqrt( (x1-x0)*(x1-x0) + (y1-y0)*(y1-y0) );
}



//////////////

Vec2 = function(x,y) {
    this.x = x;
    this.y = y;
}
Vec2.prototype.setWith2args = function(x,y) {
    if(y==undefined) {
        if( (typeof x) == "number" ) {
            this.x=x;
            this.y=x;
        } else if( x.__proto__ == Vec2.prototype ) {
            this.x=x.x;
            this.y=x.y;            
        }
    } else {
        this.x=x;
        this.y=y;
    }
}
Vec2.prototype.normalize = function(l) {
    var ll = Math.sqrt(this.x*this.x+this.y*this.y);
    if(ll==0) return new Vec2(0,0);
    return new Vec2(this.x/ll*l,this.y/ll*l);
}
Vec2.prototype.add = function(to_add) {
    return new Vec2( this.x + to_add.x, this.y + to_add.y );
}
Vec2.prototype.mul = function(to_mul) {
    return new Vec2( this.x * to_mul, this.y * to_mul );
}
Vec2.prototype.randomize = function(r) {
    return new Vec2( this.x - r + range(0,r*2), this.y - r + range(0,r*2) );
}


// 0 ~ 1
Color = function(r,g,b,a) {
    if(g==undefined || g==null) {
        var code = r; // color code
        this.r = ((code & 0xff0000)>>16)/255;
        this.g = ((code & 0xff00)>>8)/255;
        this.b = (code & 0xff)/255;
        this.a = 1.0;        
    } else {
        this.r = r;
        this.g = g;
        this.b = b;
        this.a = a;
    }
}
Color.prototype.toRGBA = function() {
    return [ parseInt(this.r*255), parseInt(this.g*255), parseInt(this.b*255), parseInt(this.a*255) ];
}
Color.prototype.toCode = function() {
    return ( parseInt(this.r * 255) << 16 ) + ( parseInt(this.g * 255) << 8 ) + parseInt(this.b * 255);
}
Color.prototype.toTHREEColor = function() {
    return new THREE.Color(this.toCode());
}
Color.prototype.equals = function(r,g,b,a) {
    return (this.r==r && this.g==g && this.b==b && this.a==a);
}
Color.prototype.adjust = function(v) {
    var rr = this.r*v;
    var gg = this.g*v;
    var bb = this.b*v;
    if(rr>1)rr=1;
    if(gg>1)gg=1;
    if(bb>1)bb=1;
    return new Color(rr,gg,bb,this.a);
}

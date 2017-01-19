//

function MoyaiClient(w,h,pixratio){
    this.scene = new THREE.Scene();
    this.renderer = new THREE.WebGLRenderer();
    this.renderer.setPixelRatio( pixratio);
    this.renderer.setSize(w,h);
    this.renderer.autoClear = false;
}
MoyaiClient.prototype.resize = function(w,h) {
    renderer.setSize(w,h);
}
MoyaiClient.prototype.render = function() {
    console.log("render");
}

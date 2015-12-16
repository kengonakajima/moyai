//
//  Shader.fsh
//  ios_demo2d
//
//  Created by ringo on 12/16/15.
//  Copyright © 2015 ringo. All rights reserved.
//

varying lowp vec4 colorVarying;

void main()
{
    gl_FragColor = colorVarying;
}

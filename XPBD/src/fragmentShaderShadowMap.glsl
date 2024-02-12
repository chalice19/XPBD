#version 330 core            // minimal GL version support expected from the GPU

// output data
layout(location=0) out float fragmentDepth;

void main(){
  // not really needed, OpenGL does it anyway
  fragmentDepth = gl_FragCoord.z;
}

#ifndef CAMERA_H
#define CAMERA_H

// Basic camera model
class Camera {
public:
  const glm::vec3& getPosition() const { return _pos; }
  void setPosition(const glm::vec3 &t) { _pos = t; }
  const glm::vec3& getRotation() const { return _rotation; }
  void setRotation(const glm::vec3 &r) { _rotation = r; }
  float getFov() const { return _fov; }
  void setFoV(float f) { _fov = f; }
  float getAspectRatio() const { return _aspectRatio; }
  void setAspectRatio(float a) { _aspectRatio = a; }
  float getNear() const { return _near; }
  void setNear(float n) { _near = n; }
  float getFar() const { return _far; }
  void setFar(float n) { _far = n; }

  glm::mat4 computeViewMatrix() const {
    glm::mat4 rot = glm::rotate(glm::mat4(1.0), _rotation[0], glm::vec3(1.0, 0.0, 0.0));
    rot = glm::rotate(rot, _rotation[1], glm::vec3(0.0, 1.0, 0.0));
    rot = glm::rotate(rot, _rotation[2], glm::vec3(0.0, 0.0, 1.0));
    const glm::mat4 trn = glm::translate(glm::mat4(1.0), _pos);
    return glm::inverse(rot*trn);
    // return glm::lookAt(_pos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
  }

  // Returns the projection matrix stemming from the camera intrinsic parameter.
  glm::mat4 computeProjectionMatrix() const {
    return glm::perspective(glm::radians(_fov), _aspectRatio, _near, _far);
  }

private:
  glm::vec3 _pos = glm::vec3(0, 0, -10);
  glm::vec3 _rotation = glm::vec3(0, 0, 0);

  float _fov = 45.f;        // Field of view, in degrees
  float _aspectRatio = 1.f; // Ratio between the width and the height of the image
  float _near = 0.1f; // Distance before which geometry is excluded fromt he rasterization process
  float _far = 10.f; // Distance after which the geometry is excluded fromt he rasterization process
};

#endif  // CAMERA_H

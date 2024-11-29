#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Transform {
public:
    glm::mat4 matrix{1.0f};
    
    glm::vec3 applyToPoint(const glm::vec3& p) const {
        return glm::vec3(matrix * glm::vec4(p, 1.0f));
    }
    
    glm::vec3 applyInverseToPoint(const glm::vec3& p) const {
        return glm::vec3(glm::inverse(matrix) * glm::vec4(p, 1.0f));
    }
};

class Camera {
protected:
    float left, right, bottom, top, near, far;
    Transform transform;
    float width, height, depth;
    
public:
    Camera(float l, float r, float b, float t, float n, float f) 
        : left(l), right(r), bottom(b), top(t), near(n), far(f) {
        width = right - left;
        height = top - bottom;
        depth = far - near;
    }
    
    float ratio() const { return width / height; }
    virtual glm::mat4 getProjectionMatrix() const = 0;
};

class OrthoCamera : public Camera {
private:
    glm::mat4 orthoTransform;
    
public:
    OrthoCamera(float l, float r, float b, float t, float n, float f) 
        : Camera(l, r, b, t, n, f) {
        orthoTransform = glm::ortho(left, right, bottom, top, near, far);
    }
    
    glm::mat4 getProjectionMatrix() const override {
        return orthoTransform;
    }
    
    glm::vec3 projectPoint(const glm::vec3& p) {
        glm::vec3 cameraSpacePoint = transform.applyInverseToPoint(p);
        glm::vec4 orthoPoint = orthoTransform * glm::vec4(cameraSpacePoint, 1.0f);
        return glm::vec3(orthoPoint);
    }
    
    glm::vec3 inverseProjectPoint(const glm::vec3& p) {
        glm::vec4 homogeneousPoint(p, 1.0f);
        glm::vec4 cameraSpacePoint = glm::inverse(orthoTransform) * homogeneousPoint;
        return transform.applyToPoint(glm::vec3(cameraSpacePoint) * glm::vec3(-1, -1, 1));
    }
};

class PerspectiveCamera : public Camera {
private:
    glm::mat4 perspectiveTransform;
    
public:
    PerspectiveCamera(float l, float r, float b, float t, float n, float f) 
        : Camera(l, r, b, t, n, f) {
        perspectiveTransform = glm::frustum(left, right, bottom, top, near, far);
    }
    
    glm::mat4 getProjectionMatrix() const override {
        return perspectiveTransform;
    }
    
    glm::vec3 projectPoint(const glm::vec3& p) {
        glm::vec3 cameraSpacePoint = transform.applyInverseToPoint(p);
        glm::vec4 perspPoint = perspectiveTransform * glm::vec4(cameraSpacePoint, 1.0f);
        return glm::vec3(perspPoint) / perspPoint.w;
    }
    
    glm::vec3 inverseProjectPoint(const glm::vec3& p) {
        glm::vec4 homogeneousPoint(p, 1.0f);
        glm::vec4 cameraSpacePoint = glm::inverse(perspectiveTransform) * homogeneousPoint;
        cameraSpacePoint /= cameraSpacePoint.w;
        return transform.applyToPoint(glm::vec3(cameraSpacePoint) * glm::vec3(-1, -1, 1));
    }
    
    static PerspectiveCamera fromFOV(float fov, float near, float far, float ratio) {
        float height = near * tan(glm::radians(fov) / 2.0f);
        float width = height * ratio;
        return PerspectiveCamera(-width, width, -height, height, near, far);
    }
};
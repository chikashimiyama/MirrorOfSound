#pragma once
#include <functional>
#include <list>
#include <chrono>

using namespace std::chrono;
class Interpolation{
public:
    Interpolation(const std::function<float(float)> &transferFunction);
    float getY(const float &x) const;
protected:
    const std::function<float(float)> transferFunction;
};

inline Interpolation::Interpolation(const std::function<float(float)> &transferFunction):
transferFunction(transferFunction)
{}

inline float Interpolation::getY(const float &x) const{
    return transferFunction(x);
}

class Animation:public Interpolation{
public:

    Animation(const std::function<float(float)> &transferFunction,
              float duration,
              const std::function<void(float)> &updateAction);
    void update();
    const float &getDuration()const;
    const float &getElapsedTime()const;
    const bool &getDone() const;
    const float &getValue()const;

protected:
    float duration; // sec
    const std::function<void(float)> updateAction;
    float elapsedTime;
    float getProgress() const;
    float value;
    const steady_clock::time_point birthTime;
    bool done;
};

inline Animation::Animation(const std::function<float(float)> &transferFunction,
                            float duration,
                            const std::function<void(float)> &updateAction):
Interpolation(transferFunction),
duration(duration),
updateAction(updateAction),
birthTime(steady_clock::now()),
done(false)
{}

inline void Animation::update(){

    if(done) return;

    steady_clock::time_point currentTime = steady_clock::now();
    std::chrono::duration<float> time_span =
            duration_cast<std::chrono::duration<float>>(currentTime - birthTime);
    elapsedTime = time_span.count();

    if(elapsedTime > duration){
        done = true;
        return;
    }
    float progress = getProgress();
    value = getY(progress);
    updateAction(progress);
}

inline const float &Animation::getDuration()const{
    return duration;
}

inline const float &Animation::getElapsedTime()const{
    return elapsedTime;
}

inline float Animation::getProgress()const{
    return elapsedTime / duration;
}

inline const bool &Animation::getDone()const{
    return done;
}

inline const float &Animation::getValue()const{
    return value;
}


class AnimationManager{
public:
    void update();
    void addAnimation(const Animation &animation);
    size_t getNumberOfActiveAnimation() const;
protected:
    std::list<Animation> activeAnimationList;
};

inline void AnimationManager::update(){
    for(Animation & anim : activeAnimationList){
          anim.update();
    }
    activeAnimationList.remove_if([](const Animation & anim){ return anim.getDone(); });
}

inline void AnimationManager::addAnimation(const Animation &animation){
    activeAnimationList.push_back(animation);
}

inline size_t AnimationManager::getNumberOfActiveAnimation() const{
    return activeAnimationList.size();
}


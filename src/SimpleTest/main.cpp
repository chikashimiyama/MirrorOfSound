#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include <catch.hpp>
#include <Animation.hpp>

TEST_CASE("Interpolation"){

    Interpolation intp([](float x){return x;});
    REQUIRE(intp.getY(0.5) == 0.5);
    Interpolation intp2([](float x){return -x;});
    REQUIRE(intp2.getY(0.5) == -0.5);
}


TEST_CASE("Animation"){
    Animation anim([](float x){return x;}, 10.0, [](const float &x){std::cout << x << std::endl;});
    do{
        anim.update();
        std::cout << "elapsed:" << anim.getElapsedTime() << std::endl;
        std::cout << "value:" << anim.getValue() << std::endl;
        sleep(1);
    }while(!anim.getDone());
}

TEST_CASE("AnimationManager"){
    AnimationManager animMan;
    animMan.addAnimation(Animation([](float x){return x;}, 5.0, [](const float &x){std::cout << x << std::endl;}));
    animMan.addAnimation(Animation([](float x){return x;}, 10.0, [](const float &x){std::cout << x << std::endl;}));

    do{
        animMan.update();
        sleep(1);
    }while(animMan.getNumberOfActiveAnimation() > 0);
}

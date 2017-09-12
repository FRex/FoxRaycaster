#include "FoxRaycaster.hpp"
#include <SFML/Graphics.hpp>
#include <sstream>
#include <algorithm>

const unsigned kResolutionsCount = 5u;

const sf::Vector2u kResolutions[kResolutionsCount] = {
    {320u, 240u},
    {640u, 480u},
    {800u, 600u},
    {1024u, 768u},
    {1280u, 1024u},
};

class RunInfo
{
public:
    std::string toString() const
    {
        std::ostringstream ss;
        ss << std::boolalpha;
        ss << "FPS: " << fps << std::endl;
        ss << "(R) Stretch:    " << stretch << std::endl;
        ss << "(T) Smooth:     " << smooth << std::endl;
        ss << "(Y) Resolution: " << kResolutions[resolution].x << 'x' << kResolutions[resolution].y << std::endl;
        ss << "(U) Depthdraw:  " << depthdraw << std::endl;
        return ss.str();
    }

    float fps = 0.f;
    bool stretch = false;
    bool smooth = false;
    unsigned resolution = 0u;
    bool depthdraw = false;

};

int main(int argc, char ** argv)
{
    RunInfo runinfo;

    sf::RenderWindow app(sf::VideoMode(800u, 600u), "FoxRaycaster");
    app.setFramerateLimit(60u);

    fox::FoxRaycaster raycaster;
    raycaster.setScreenSize(kResolutions[runinfo.resolution].x, kResolutions[runinfo.resolution].y);

    sf::Image img;
    if(img.loadFromFile("tex1.png"))
        raycaster.setTexture(1u, img);

    if(img.loadFromFile("tex2.png"))
        raycaster.setTexture(2u, img);

    if(img.loadFromFile("map.png"))
    {
        raycaster.setMapSize(img.getSize().x, img.getSize().y);
        for(unsigned x = 0u; x < img.getSize().x; ++x)
            for(unsigned y = 0u; y < img.getSize().y; ++y)
                raycaster.setMapTile(x, y, img.getPixel(x, y) != sf::Color::Black);
    }

    sf::Texture tex;
    sf::Font font;
    font.loadFromFile("DejaVuSansMono.ttf");
    sf::Clock fpsclock;
    unsigned frames = 0u;
    while(app.isOpen())
    {
        sf::Event eve;
        while(app.pollEvent(eve))
        {
            switch(eve.type)
            {
            case sf::Event::Closed:
                app.close();
                break;
            case sf::Event::Resized:
                //raycaster.setScreenSize(eve.size.width, eve.size.height);
                app.setView(sf::View(sf::FloatRect(0.f, 0.f, eve.size.width, eve.size.height)));
                break;
            case sf::Event::KeyPressed:
                switch(eve.key.code)
                {
                case sf::Keyboard::R:
                    runinfo.stretch = !runinfo.stretch;
                    break;
                case sf::Keyboard::T:
                    runinfo.smooth = !runinfo.smooth;
                    break;
                case sf::Keyboard::Y:
                    runinfo.resolution = (runinfo.resolution + 1) % kResolutionsCount;
                    raycaster.setScreenSize(kResolutions[runinfo.resolution].x, kResolutions[runinfo.resolution].y);
                    break;
                case sf::Keyboard::U:
                    runinfo.depthdraw = !runinfo.depthdraw;
                    break;
                }//switch eve key code
                break;
            }
        }//while app poll event eve

        app.clear(sf::Color(0x2d0022ff));
        raycaster.handleKeys();
        raycaster.rasterize();
        if(runinfo.depthdraw)
        {
            tex.loadFromImage(raycaster.getDepthImage());
        }
        else
        {
            tex.loadFromImage(raycaster.getImage());
        }

        sf::Sprite spr(tex);
        tex.setSmooth(runinfo.smooth);
        if(runinfo.stretch)
        {
            const auto ts = sf::Vector2f(tex.getSize());
            const auto ws = sf::Vector2f(app.getSize());
            const float s = std::min(ws.x / ts.x, ws.y / ts.y);
            spr.setScale(s, s);
        }//if runinfo stretch

        spr.setOrigin(sf::Vector2f(tex.getSize()) / 2.f);
        spr.setPosition(sf::Vector2f(app.getSize()) / 2.f);
        app.draw(spr);

        sf::Text txt(runinfo.toString(), font);
        txt.setOutlineThickness(1.f);
        txt.setFillColor(sf::Color::White);
        txt.setOutlineColor(sf::Color::Black);
        app.draw(txt);
        app.display();

        ++frames;
        if(frames > 60u || fpsclock.getElapsedTime().asSeconds() > 1.5f)
        {
            runinfo.fps = frames / fpsclock.getElapsedTime().asSeconds();
            frames = 0u;
            fpsclock.restart();
        }//if need to poll and reset fps counter
    }
    return 0;
}

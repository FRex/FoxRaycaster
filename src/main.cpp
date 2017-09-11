#include "FoxRaycaster.hpp"
#include <SFML/Graphics.hpp>

int main(int argc, char ** argv)
{
    sf::RenderWindow app(sf::VideoMode(800u, 600u), "FoxRaycaster");
    app.setFramerateLimit(60u);

    fox::FoxRaycaster raycaster;

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
                raycaster.setMapTile(x,y, img.getPixel(x, y) != sf::Color::Black);
    }

    sf::Texture tex;
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
                raycaster.setScreenSize(eve.size.width, eve.size.height);
                app.setView(sf::View(sf::FloatRect(0.f, 0.f, eve.size.width, eve.size.height)));
                break;
            }
        }//while app poll event eve

        app.clear();
        raycaster.handleKeys();
        sf::Clock clo;
        raycaster.rasterize();
        std::printf("%ums\n", clo.getElapsedTime().asMilliseconds());
        tex.loadFromImage(raycaster.getImage());
        app.draw(sf::Sprite(tex));
        app.display();
    }

    return 0;
}

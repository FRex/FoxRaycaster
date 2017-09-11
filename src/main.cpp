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
            }
        }

        app.clear();
        raycaster.handleKeys();
        raycaster.rasterize();
        tex.loadFromImage(raycaster.getImage());
        app.draw(sf::Sprite(tex));
        app.display();
    }

    return 0;
}

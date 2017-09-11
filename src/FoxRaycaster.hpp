#pragma once
#include <SFML/Graphics/Image.hpp>

namespace fox {

class FoxRaycaster
{
public:
    FoxRaycaster();
    void rasterize();
    const sf::Image& getImage() const;
    void handleKeys();
    void setTexture(unsigned texnum, const sf::Image& img);

private:
    unsigned * getTexture(unsigned num);
    const unsigned * getTexture(unsigned num) const;

    float m_camposx = 22.f;
    float m_camposy = 12.f;
    float m_dirx = -1.f;
    float m_diry = 0.f;
    float m_planex = 0.f;
    float m_planey = 0.66f;

    std::vector<unsigned> m_screen;
    std::vector<unsigned> m_textures;
    std::vector<sf::Uint8> m_sfbuffer;
    sf::Image m_sfimage;

};

}//fox

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
    void setScreenSize(unsigned width, unsigned height);
    void setMapSize(unsigned width, unsigned height);
    void setMapTile(unsigned x, unsigned y, unsigned tile);

private:
    unsigned * getTexture(unsigned num);
    const unsigned * getTexture(unsigned num) const;
    unsigned screenPixelIndex(unsigned x, unsigned y);
    unsigned getMapTile(unsigned x, unsigned y) const;

    float m_camposx = 5.f;
    float m_camposy = 5.f;
    float m_dirx = -1.f;
    float m_diry = 0.f;
    float m_planex = 0.f;
    float m_planey = 0.66f;

    std::vector<unsigned> m_screen;
    std::vector<unsigned> m_textures;
    std::vector<sf::Uint8> m_sfbuffer;
    sf::Image m_sfimage;
    unsigned m_screenwidth;
    unsigned m_screenheight;
    unsigned m_screenpixels;
    std::vector<unsigned> m_map;
    unsigned m_mapwidth;
    unsigned m_mapheight;

};

}//fox

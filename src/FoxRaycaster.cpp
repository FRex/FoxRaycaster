#include "FoxRaycaster.hpp"
#include <cmath>
#include <SFML/Window/Keyboard.hpp>

/*
Original raycasting code from tutorials at: http://lodev.org/cgtutor/index.html

Copyright (c) 2004-2007, Lode Vandevenne

All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

namespace fox {

const unsigned kTextureSize = 64u;
const unsigned kTexturePixels = kTextureSize * kTextureSize;
const float kClearDepth = 0.f;

inline unsigned texturePixelIndex(unsigned x, unsigned y)
{
    return x + kTextureSize * y;
}

inline unsigned halveRGB(unsigned color)
{
    const unsigned char r = ((color >> 24) & 0xff) / 2;
    const unsigned char g = ((color >> 16) & 0xff) / 2;
    const unsigned char b = ((color >> 8) & 0xff) / 2;
    const unsigned char a = color & 0xff;
    return (r << 24) + (g << 16) + (b << 8) + a;
}

inline unsigned complementRGB(unsigned color)
{
    const unsigned char r = 255 - ((color >> 24) & 0xff);
    const unsigned char g = 255 - ((color >> 16) & 0xff);
    const unsigned char b = 255 - ((color >> 8) & 0xff);
    const unsigned char a = color & 0xff;
    return (r << 24) + (g << 16) + (b << 8) + a;
}

FoxRaycaster::FoxRaycaster()
{
    setScreenSize(800u, 600u);
    setMapSize(10u, 10u);

    //jorge
    m_textures.assign(kTexturePixels, 0xff);
    unsigned * tex0 = getTexture(0u);
    for(int x = 0; x < kTextureSize; ++x)
    {
        for(int y = 0; y < kTextureSize; ++y)
        {
            const int xx = x / 8;
            const int yy = y / 8;
            if((xx + yy) % 2 == 0)
            {
                tex0[texturePixelIndex(x, y)] = 0xff00ffff;
            }
            else
            {
                tex0[texturePixelIndex(x, y)] = 0x00007fff;
            }

            if(x - std::abs(static_cast<int>(kTextureSize) - 2 * y) > 0)
                tex0[texturePixelIndex(x, y)] = complementRGB(tex0[texturePixelIndex(x, y)]);
        }//for y
    }//for x
}

void FoxRaycaster::rasterize()
{
    m_screen.assign(m_screenpixels, 0x7f7f7fff);
    m_depthbuffer.assign(m_screenpixels, kClearDepth);

    for(int x = 0; x < m_screenwidth; ++x)
    {
        //calculate ray position and direction
        const float camerax = 2.f * x / static_cast<float>(m_screenwidth) - 1.f; //x-coordinate in camera space
        const float rayposx = m_camposx;
        const float rayposy = m_camposy;
        const float raydirx = m_dirx + m_planex * camerax;
        const float raydiry = m_diry + m_planey * camerax;

        //which box of the map we're in
        int mapx = static_cast<int>(rayposx);
        int mapy = static_cast<int>(rayposy);

        //length of ray from current position to next x or y-side
        float sidedistx;
        float sidedisty;

        //length of ray from one x or y-side to next x or y-side
        const float deltadistx = std::sqrt(1 + (raydiry * raydiry) / (raydirx * raydirx));
        const float deltadisty = std::sqrt(1 + (raydirx * raydirx) / (raydiry * raydiry));
        float perpwalldist;

        //what direction to step in x or y-direction (either +1 or -1)
        int stepx;
        int stepy;

        int hit = 0; //was there a wall hit?
        int side; //was a NS or a EW wall hit?
                  //calculate step and initial sideDist
        if(raydirx < 0)
        {
            stepx = -1;
            sidedistx = (rayposx - mapx) * deltadistx;
        }
        else
        {
            stepx = 1;
            sidedistx = (mapx + 1.f - rayposx) * deltadistx;
        }
        if(raydiry < 0)
        {
            stepy = -1;
            sidedisty = (rayposy - mapy) * deltadisty;
        }
        else
        {
            stepy = 1;
            sidedisty = (mapy + 1.f - rayposy) * deltadisty;
        }

        //perform DDA
        while(hit == 0)
        {
            //jump to next map square, OR in x-direction, OR in y-direction
            if(sidedistx < sidedisty)
            {
                sidedistx += deltadistx;
                mapx += stepx;
                side = 0;
            }
            else
            {
                sidedisty += deltadisty;
                mapy += stepy;
                side = 1;
            }
            //Check if ray has hit a wall
            hit = getMapTile(mapx, mapy) > 0u;
        }

        //Calculate distance projected on camera direction (oblique distance will give fisheye effect!)
        if(side == 0)
            perpwalldist = (mapx - rayposx + (1 - stepx) / 2) / raydirx;
        else
            perpwalldist = (mapy - rayposy + (1 - stepy) / 2) / raydiry;

        //Calculate height of line to draw on screen
        const int lineheight = static_cast<int>(m_screenheight / perpwalldist);

        //calculate lowest and highest pixel to fill in current stripe
        int drawstart = -lineheight / 2 + m_screenheight / 2;
        if(drawstart < 0)
            drawstart = 0;

        int drawend = lineheight / 2 + m_screenheight / 2;
        if(drawend >= m_screenheight)
            drawend = m_screenheight - 1;

        //choose wall color
        if(getMapTile(mapx, mapy) > 0)
        {
            float wallx;
            if(side == 0)
                wallx = rayposy + perpwalldist * raydiry;
            else
                wallx = rayposx + perpwalldist * raydirx;

            wallx -= std::floor((wallx));

            int texx = static_cast<int>(wallx * static_cast<float>(kTextureSize));
            if(side == 0 && raydirx > 0)
                texx = kTextureSize - texx - 1;

            if(side == 1 && raydiry < 0)
                texx = kTextureSize - texx - 1;

            for(int y = drawstart; y < drawend; y++)
            {
                const int d = y * 256 - m_screenheight * 128 + lineheight * 128;  //256 and 128 factors to avoid floats
                const int texy = ((d * kTextureSize) / lineheight) / 256;
                const unsigned * tex0 = getTexture(getMapTile(mapx, mapy));

                unsigned color = tex0[texturePixelIndex(texx, texy)];
                if(side == 1)
                    color = halveRGB(color);

                m_screen[screenPixelIndex(x, y)] = color;
                m_depthbuffer[screenPixelIndex(x, y)] = perpwalldist;
            }//for y

            //FLOOR CASTING:
            float floorxwall, floorywall;

            //4 different wall directions possible
            if(side == 0 && raydirx > 0.f)
            {
                floorxwall = static_cast<float>(mapx);
                floorywall = static_cast<float>(mapy + wallx);
            }
            else if(side == 0 && raydirx < 0.f)
            {
                floorxwall = mapx + 1.f;
                floorywall = mapy + wallx;
            }
            else if(side == 1 && raydiry > 0.f)
            {
                floorxwall = mapx + wallx;
                floorywall = static_cast<float>(mapy);
            }
            else
            {
                floorxwall = mapx + wallx;
                floorywall = mapy + 1.f;
            }

            float distwall, distplayer;
            distwall = perpwalldist;
            distplayer = 0.f;

            if(drawend < 0)
                drawend = m_screenheight; //becomes < 0 when the integer overflows

            //draw the floor from drawEnd to the bottom of the screen
            for(int y = drawend + 1; y < m_screenheight; ++y)
            {
                const float currentdist = m_screenheight / (2.f * y - m_screenheight); //you could make a small lookup table for this instead
                const float weight = (currentdist - distplayer) / (distwall - distplayer);
                const float currentfloorx = weight * floorxwall + (1.f - weight) * m_camposx;
                const float currentfloory = weight * floorywall + (1.f - weight) * m_camposy;
                const int floortexx = static_cast<int>(currentfloorx * kTextureSize) % kTextureSize;
                const int floortexy = static_cast<int>(currentfloory * kTextureSize) % kTextureSize;


                const unsigned * floortex = getTexture(2u);
                const unsigned * ceiltex = getTexture(0u);

                if((static_cast<int>(currentfloorx) + static_cast<int>(currentfloory)) % 2)
                    std::swap(floortex, ceiltex);

                //floor
                m_screen[screenPixelIndex(x, y)] = floortex[texturePixelIndex(floortexx, floortexy)];
                m_depthbuffer[screenPixelIndex(x, y)] = currentdist;

                //ceiling (symmetrical!)
                m_screen[screenPixelIndex(x, m_screenheight - y)] = ceiltex[texturePixelIndex(floortexx, floortexy)];
                m_depthbuffer[screenPixelIndex(x, m_screenheight - y)] = currentdist;
            }


        }//if world map > 0
    }//for x

    //commit to sf image
    m_sfbuffer.resize(m_screenpixels * 4u);
    for(unsigned i = 0; i < m_screen.size(); ++i)
    {
        m_sfbuffer[i * 4 + 0] = (m_screen[i] >> 24) & 0xff;
        m_sfbuffer[i * 4 + 1] = (m_screen[i] >> 16) & 0xff;
        m_sfbuffer[i * 4 + 2] = (m_screen[i] >> 8) & 0xff;
        m_sfbuffer[i * 4 + 3] = (m_screen[i] >> 0) & 0xff;
    }//for i

    m_sfimage.create(m_screenwidth, m_screenheight, m_sfbuffer.data());
    std::vector<sf::Uint8> greydepthpixels;
    const float maxdepth = *std::max_element(m_depthbuffer.begin(), m_depthbuffer.end());
    for(float f : m_depthbuffer)
    {
        const float x = 255.f * (f / maxdepth);
        greydepthpixels.push_back(x);
        greydepthpixels.push_back(x);
        greydepthpixels.push_back(x);
        greydepthpixels.push_back(255u);
    }//for each f in m depth buffer
    m_depthimage.create(m_screenwidth, m_screenheight, greydepthpixels.data());
}

const sf::Image& FoxRaycaster::getImage() const
{
    return m_sfimage;
}

void FoxRaycaster::handleKeys()
{
    //speed modifiers
    const float boost = 2.f;
    const float movespeed = boost * (5.f / 60.f);
    const float rotspeed = boost * (3.f / 60.f);

    //move forward if no wall in front of you
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::W))
    {
        if(getMapTile(m_camposx + m_dirx * movespeed, m_camposy) == 0)
            m_camposx += m_dirx * movespeed;

        if(getMapTile(m_camposx, m_camposy + m_diry * movespeed) == 0)
            m_camposy += m_diry * movespeed;
    }

    //move backwards if no wall behind you
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::S))
    {
        if(getMapTile(m_camposx - m_dirx * movespeed, m_camposy) == 0)
            m_camposx -= m_dirx * movespeed;

        if(getMapTile(m_camposx, m_camposy - m_diry * movespeed) == 0)
            m_camposy -= m_diry * movespeed;
    }

    //rotate to the right
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::D))
    {
        //both camera direction and camera plane must be rotated
        const float olddirx = m_dirx;
        m_dirx = m_dirx * std::cos(-rotspeed) - m_diry * sin(-rotspeed);
        m_diry = olddirx * std::sin(-rotspeed) + m_diry * cos(-rotspeed);
        const float oldplanex = m_planex;
        m_planex = m_planex * cos(-rotspeed) - m_planey * std::sin(-rotspeed);
        m_planey = oldplanex * sin(-rotspeed) + m_planey * std::cos(-rotspeed);
    }

    //rotate to the left
    if(sf::Keyboard::isKeyPressed(sf::Keyboard::A))
    {
        //both camera direction and camera plane must be rotated
        float oldDirX = m_dirx;
        m_dirx = m_dirx * std::cos(rotspeed) - m_diry * std::sin(rotspeed);
        m_diry = oldDirX * std::sin(rotspeed) + m_diry * std::cos(rotspeed);
        const float oldplanex = m_planex;
        m_planex = m_planex * std::cos(rotspeed) - m_planey * sin(rotspeed);
        m_planey = oldplanex * std::sin(rotspeed) + m_planey * cos(rotspeed);
    }
}

void FoxRaycaster::setTexture(unsigned texnum, const sf::Image& img)
{
    if(img.getSize() != sf::Vector2u(kTextureSize, kTextureSize))
        return;

    if((texnum * kTexturePixels) >= m_textures.size())
        m_textures.resize((texnum + 1u) * kTexturePixels, 0xff00ffff);

    unsigned * t = getTexture(texnum);
    for(int x = 0; x < kTextureSize; ++x)
        for(int y = 0; y < kTextureSize; ++y)
            t[texturePixelIndex(x, y)] = img.getPixel(x, y).toInteger();
}

void FoxRaycaster::setScreenSize(unsigned width, unsigned height)
{
    if(m_screenwidth == width && m_screenheight == height)
        return;

    height = height - (height % 2); //only even height allowed
    m_screenwidth = width;
    m_screenheight = height;
    m_screenpixels = width * height;
    m_screen.assign(m_screenpixels, 0x7f7f7fff);
    m_depthbuffer.assign(m_screenpixels, kClearDepth);
}

void FoxRaycaster::setMapSize(unsigned width, unsigned height)
{
    if(width < 3u || height < 3u)
        return;

    m_mapwidth = width;
    m_mapheight = height;
    m_map.assign(width * height, 0u);

    for(unsigned x = 0; x < width; ++x)
    {
        setMapTile(x, 0u, 1u);
        setMapTile(x, height - 1u, 1u);
    }

    for(unsigned y = 0; y < height; ++y)
    {
        setMapTile(0u, y, 1u);
        setMapTile(width - 1u, y, 1u);
    }
}

void FoxRaycaster::setMapTile(unsigned x, unsigned y, unsigned tile)
{
    if(x < m_mapwidth && y < m_mapheight)
        m_map[x + y * m_mapwidth] = tile;
}

const sf::Image& FoxRaycaster::getDepthImage() const
{
    return m_depthimage;
}

unsigned * FoxRaycaster::getTexture(unsigned texnum)
{
    if((texnum * kTexturePixels) < m_textures.size())
        return m_textures.data() + texnum * kTexturePixels;

    return m_textures.data(); //jorge
}

const unsigned * FoxRaycaster::getTexture(unsigned texnum) const
{
    if((texnum * kTexturePixels) < m_textures.size())
        return m_textures.data() + texnum * kTexturePixels;

    return m_textures.data(); //jorge
}

unsigned FoxRaycaster::screenPixelIndex(unsigned x, unsigned y)
{
    return x + m_screenwidth * y;
}

unsigned FoxRaycaster::getMapTile(unsigned x, unsigned y) const
{
    if(x < m_mapwidth && y < m_mapheight)
        return m_map[x + m_mapwidth * y];

    return 1u;
}

}//fox

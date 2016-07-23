/* This file is part of RetroFE.
 *
 * RetroFE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * RetroFE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with RetroFE.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ScrollingText.h"
#include "../ViewInfo.h"
#include "../../Database/Configuration.h"
#include "../../Utility/Log.h"
#include "../../Utility/Utils.h"
#include "../../SDL.h"
#include "../Font.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>
#include <algorithm>


ScrollingText::ScrollingText(Configuration &config, bool systemMode, bool layoutMode, std::string type, std::string textFormat, std::string alignment, Page &p, int displayOffset, Font *font, float scaleX, float scaleY, std::string direction, float scrollingSpeed, float startPosition, float startTime, float endTime )
    : Component(p)
    , config_(config)
    , systemMode_(systemMode)
    , layoutMode_(layoutMode)
    , fontInst_(font)
    , type_(type)
    , textFormat_(textFormat)
    , alignment_(alignment)
    , scaleX_(scaleX)
    , scaleY_(scaleY)
    , direction_(direction)
    , scrollingSpeed_(scrollingSpeed)
    , startPosition_(startPosition)
    , currentPosition_(-startPosition)
    , startTime_(startTime)
    , waitStartTime_(startTime)
    , endTime_(endTime)
    , waitEndTime_(0.0f)
    , currentCollection_("")
    , page_(NULL)
    , displayOffset_(displayOffset)

{
    text_.clear( );
}


ScrollingText::~ScrollingText( )
{
}


void ScrollingText::update(float dt)
{

    if (waitEndTime_ > 0)
    {
        waitEndTime_ -= dt;
    }
    else if (waitStartTime_ > 0)
    {
        waitStartTime_ -= dt;
    }
    else
    {
        if (direction_ == "horizontal")
        {
            currentPosition_ += scrollingSpeed_ * dt * scaleX_;
        }
        else if (direction_ == "vertical")
        {
            currentPosition_ += scrollingSpeed_ * dt * scaleY_;
        }
    }

    if (newItemSelected)
    {
        reloadTexture( );
        newItemSelected = false;
    }

    Component::update(dt);
}


void ScrollingText::freeGraphicsMemory( )
{
    Component::freeGraphicsMemory( );
    text_.clear( );
}


void ScrollingText::reloadTexture( )
{

    if (direction_ == "horizontal")
    {
        currentPosition_ = -startPosition_ * scaleX_;
    }
    else if (direction_ == "vertical")
    {
        currentPosition_ = -startPosition_ * scaleY_;
    }
    waitStartTime_   = startTime_;
    waitEndTime_     = 0.0f;

    text_.clear( );

    Item *selectedItem = page.getSelectedItem( displayOffset_ );
    if (!selectedItem)
    {
        return;
    }

    config_.getProperty( "currentCollection", currentCollection_ );

    // build clone list
    std::vector<std::string> names;

    names.push_back( selectedItem->name );
    names.push_back( selectedItem->fullTitle );

    if (selectedItem->cloneof.length( ) > 0)
    {
        names.push_back( selectedItem->cloneof );
    }

    // Check for corresponding .txt files
    for (unsigned int n = 0; n < names.size( ) && text_.empty( ); ++n)
    {

        std::string basename = names[n];

        Utils::replaceSlashesWithUnderscores( basename );

        if (systemMode_)
        {

            // check the master collection for the system artifact 
            loadText( collectionName, type_, type_, true );

            // check collection for the system artifact
            if (text_.empty( ))
            {
              loadText( selectedItem->collectionInfo->name, type_, type_, true );
            }

        }
        else
        {

            // are we looking at a leaf or a submenu
            if (selectedItem->leaf) // item is a leaf
            {

              // check the master collection for the artifact 
              loadText( collectionName, type_, basename, false );

              // check the collection for the artifact
              if (text_.empty( ))
              {
                loadText( selectedItem->collectionInfo->name, type_, basename, false );
              }

            }
            else // item is a submenu
            {

              // check the master collection for the artifact 
              loadText( collectionName, type_, basename, false );

              // check the collection for the artifact
              if (text_.empty( ))
              {
                loadText( selectedItem->collectionInfo->name, type_, basename, false );
              }

              // check the submenu collection for the system artifact
              if (text_.empty( ))
              {
                loadText( selectedItem->name, type_, type_, true );
              } 

            }

        }
    }

}


void ScrollingText::loadText( std::string collection, std::string type, std::string basename, bool systemMode )
{

    std::string textPath = "";

    // check the system folder
    if (layoutMode_)
    {
        std::string layoutName;
        config_.getProperty("layout", layoutName);
        textPath = Utils::combinePath(Configuration::absolutePath, "layouts", layoutName, "collections", collection);
        if (systemMode)
            textPath = Utils::combinePath(textPath, "system_artwork");
        else
            textPath = Utils::combinePath(textPath, "medium_artwork", type);
    }
    else
    {
        config_.getMediaPropertyAbsolutePath( collection, type, systemMode, textPath );
    }

    textPath = Utils::combinePath( textPath, basename );

    textPath += ".txt";

    std::ifstream includeStream( textPath.c_str( ) );

    if (!includeStream.good( ))
    {
        return;
    }

    std::string line; 

    while(std::getline(includeStream, line))
    {

        // In horizontal scrolling direction, add a space before every line except the first.
        if (direction_ == "horizontal" && !text_.empty( ))
        {
            line = " " + line;
        }

        // Reformat lines to uppercase or lowercase
        if (textFormat_ == "uppercase")
        {
            std::transform(line.begin(), line.end(), line.begin(), ::toupper);
        }
        if (textFormat_ == "lowercase")
        {
            std::transform(line.begin(), line.end(), line.begin(), ::tolower);
        }

        text_.push_back( line );

    }

    return;

}


void ScrollingText::draw( )
{
    Component::draw( );

    if (!text_.empty( ) && waitEndTime_ <= 0.0f)
    {

        Font *font;
        if (baseViewInfo.font) // Use font of this specific item if available
          font = baseViewInfo.font;
        else                   // If not, use the general font settings
          font = fontInst_;

        SDL_Texture *t = font->getTexture( );

        float imageWidth     = 0;
        float imageMaxWidth  = 0;
        float imageMaxHeight = 0;
        if (baseViewInfo.Width < baseViewInfo.MaxWidth && baseViewInfo.Width > 0)
        {
            imageMaxWidth = baseViewInfo.Width;
        }
        else
        {
            imageMaxWidth = baseViewInfo.MaxWidth;
        }
        if (baseViewInfo.Height < baseViewInfo.MaxHeight && baseViewInfo.Height > 0)
        {
            imageMaxHeight = baseViewInfo.Height;
        }
        else
        {
            imageMaxHeight = baseViewInfo.MaxHeight;
        }

        float scale = (float)baseViewInfo.FontSize / (float)font->getHeight( ) / scaleY_;

        float xOrigin = baseViewInfo.XRelativeToOrigin( );
        float yOrigin = baseViewInfo.YRelativeToOrigin( );

        SDL_Rect rect;

        float position = 0.0f;

        if (direction_ == "horizontal")
        {

            rect.x = static_cast<int>( xOrigin );

            if (currentPosition_ < 0)
            {
                rect.x -= static_cast<int>( currentPosition_ );
            }

            for (unsigned int l = 0; l < text_.size( ); ++l)
            {
                for (unsigned int i = 0; i < text_[l].size( ); ++i)
                {

                    // Do not print outside the box
                    if (rect.x >= (static_cast<int>( xOrigin ) + imageMaxWidth))
                    {
                        break;
                    }

                    Font::GlyphInfo glyph;

                    if (font->getRect( text_[l][i], glyph) && glyph.rect.h > 0)
                    {
                        SDL_Rect charRect = glyph.rect;
                        rect.h  = static_cast<int>( charRect.h * scale * scaleY_ );
                        rect.w  = static_cast<int>( charRect.w * scale * scaleX_ );
                        rect.y  = static_cast<int>( yOrigin );

                        if (font->getAscent( ) < glyph.maxY)
                        {
                            rect.y += static_cast<int>( (font->getAscent( ) - glyph.maxY) * scale * scaleY_ );
                        }

                        // Check if glyph falls partially outside the box at the back end
                        if ((rect.x + static_cast<int>( glyph.advance * scale * scaleX_ )) >= (static_cast<int>( xOrigin ) + imageMaxWidth))
                        {
                            rect.w     = static_cast<int>( xOrigin ) + static_cast<int>( imageMaxWidth ) - rect.x;
                            charRect.w = static_cast<int>( rect.w / scale / scaleX_ );
                        }

                        // Print the glyph if it falls (partially) within the box
                        if ( position + glyph.advance * scale * scaleX_ > currentPosition_ )
                        {
                            // Check if glyph falls partially outside the box at the front end
                            if ( position < currentPosition_ )
                            {
                                rect.w     = static_cast<int>( glyph.advance * scale * scaleX_ + position - currentPosition_ );
                                charRect.x = static_cast<int>( charRect.x + charRect.w - rect.w / scale / scaleX_ );
                                charRect.w = static_cast<int>( rect.w / scale / scaleX_ );
                            }
                            if (rect.w > 0)
                            {
                                SDL::renderCopy(t, baseViewInfo.Alpha, &charRect, &rect, baseViewInfo);
                            }
                            rect.x += rect.w;
                        }
                        position += glyph.advance * scale * scaleX_;

                    }
                }
            }

            // Determine image width
            for (unsigned int l = 0; l < text_.size( ); ++l)
            {
                for (unsigned int i = 0; i < text_[l].size( ); ++i)
                {
                    Font::GlyphInfo glyph;
                    if (font->getRect( text_[l][i], glyph ))
                    {
                        imageWidth += glyph.advance;
                    }
                }
            }

            // Reset scrolling position when we're done
            if (currentPosition_ > imageWidth * scale * scaleX_)
            {
                waitStartTime_   = startTime_;
                waitEndTime_     = endTime_;
                currentPosition_ = -startPosition_ * scaleX_;
            }

        }
        else if (direction_ == "vertical")
        {

            unsigned int spaceWidth = 0;
            {
                Font::GlyphInfo glyph;
                if (font->getRect( ' ', glyph) )
                {
                    spaceWidth = static_cast<int>( glyph.advance * scale * scaleX_);
                }
            }

            // Reformat the text based on the image width
            std::vector<std::string>  text;
            std::vector<unsigned int> textWords;
            std::vector<unsigned int> textWidth;
            std::vector<bool>         textLast;
            for (unsigned int l = 0; l < text_.size( ); ++l)
            {
                std::string        line = "";
                std::istringstream iss(text_[l]);
                std::string        word;
                unsigned int       width     = 0;
                unsigned int       lineWidth = 0;
                unsigned int       wordCount = 0;
                while (iss >> word)
                {

                    // Determine word image width
                    unsigned int wordWidth = 0;
                    for (unsigned int i = 0; i < word.size( ); ++i)
                    {
                        Font::GlyphInfo glyph;
                        if (font->getRect( word[i], glyph) )
                        {
                            wordWidth += static_cast<int>( glyph.advance * scale * scaleX_ );
                        }
                    }
                    // Determine if the word will fit on the line
                    if (width > 0 && (width + spaceWidth + wordWidth > imageMaxWidth))
                    {
                        text.push_back( line );
                        textWords.push_back( wordCount );
                        textWidth.push_back( lineWidth );
                        textLast.push_back( false );
                        line      = word;
                        width     = wordWidth;
                        lineWidth = wordWidth;
                        wordCount = 1;
                    }
                    else
                    {
                        if (width == 0)
                        {
                            line  += word;
                            width += wordWidth;
                        }
                        else
                        {
                            line  += " " + word;
                            width += spaceWidth + wordWidth;
                        }
                        lineWidth += wordWidth;
                        wordCount += 1;
                    }
                }
                if (text_[l] == "" || line != "")
                {
                    text.push_back( line );
                    textWords.push_back( wordCount );
                    textWidth.push_back( lineWidth );
                    textLast.push_back( true );
                    width     = 0;
                    lineWidth = 0;
                    wordCount = 0;
                }
            }


            // Print reformatted text
            rect.y = static_cast<int>( yOrigin );

            if (currentPosition_ < 0)
            {
                rect.y -= static_cast<int>( currentPosition_ );
            }

            // Do not scroll if the text fits fully inside the box, and start position is 0
            if (text.size() * font->getHeight( ) * scale * scaleY_ <= imageMaxHeight && startPosition_ == 0.0f)
            {
                currentPosition_ = 0.0f;
                waitStartTime_   = 0.0f;
                waitEndTime_     = 0.0f;
            }

            for (unsigned int l = 0; l < text.size( ); ++l)
            {

                // Do not print outside the box
                if (rect.y >= (static_cast<int>( yOrigin ) + imageMaxHeight))
                {
                    break;
                }

                // Define x coordinate
                rect.x = static_cast<int>( xOrigin );
                if (alignment_ == "right")
                {
                    rect.x = static_cast<int>( xOrigin + imageMaxWidth - textWidth[l] - (textWords[l] - 1) * spaceWidth * scale * scaleX_ );
                }
                if (alignment_ == "centered")
                {
                    rect.x = static_cast<int>( xOrigin + imageMaxWidth / 2 - textWidth[l] / 2 - (textWords[l] - 1) * spaceWidth * scale * scaleX_ / 2 );
                }

                std::istringstream iss(text[l]);
                std::string        word;
                unsigned int       wordCount = textWords[l];
                unsigned int       spaceFill = static_cast<int>( imageMaxWidth ) - textWidth[l];
                unsigned int       yAdvance  = static_cast<int>( font->getHeight( ) * scale * scaleY_ );
                while (iss >> word)
                {

                    for (unsigned int i = 0; i < word.size( ); ++i)
                    {
                        Font::GlyphInfo glyph;

                        if (font->getRect( word[i], glyph) && glyph.rect.h > 0)
                        {
                            SDL_Rect charRect = glyph.rect;
                            rect.h   = static_cast<int>( charRect.h * scale * scaleY_ );
                            rect.w   = static_cast<int>( charRect.w * scale * scaleX_ );
                            yAdvance = static_cast<int>( font->getHeight( ) * scale * scaleY_ );

                            // Check if glyph falls partially outside the box at the bottom end
                            if ((rect.y + rect.h) >= (static_cast<int>( yOrigin ) + imageMaxHeight))
                            {
                                rect.h     = static_cast<int>( yOrigin ) + static_cast<int>( imageMaxHeight ) - rect.y;
                                charRect.h = static_cast<int>( rect.h / scale / scaleY_ );
                            }

                            // Print the glyph if it falls (partially) within the box
                            if ( position + font->getHeight( ) * scale * scaleY_ > currentPosition_ )
                            {
                                // Check if glyph falls partially outside the box at the front end
                                if ( position < currentPosition_ )
                                {
                                    yAdvance  -= rect.h - static_cast<int>( font->getHeight( ) * scale * scaleX_ + position - currentPosition_ );
                                    rect.h     = static_cast<int>( font->getHeight( ) * scale * scaleX_ + position - currentPosition_ );
                                    charRect.y = static_cast<int>( charRect.y + charRect.h - rect.h / scale / scaleX_ );
                                    charRect.h = static_cast<int>( rect.h / scale / scaleX_ );
                                }
                                if (rect.h > 0)
                                {
                                    SDL::renderCopy(t, baseViewInfo.Alpha, &charRect, &rect, baseViewInfo);
                                }
                            }
                            rect.x += static_cast<int>( glyph.advance * scale * scaleX_ );
                        }
                    }

                    // Print justified
                    wordCount -= 1;
                    if (wordCount > 0 && !textLast[l] && alignment_ == "justified")
                    {
                        unsigned int advance = static_cast<int>( spaceFill / wordCount );
                        spaceFill -= advance;
                        rect.x    += advance;
                    }
                    else
                    {
                        rect.x += static_cast<int>( spaceWidth * scale * scaleX_ );
                    }
                }

                // Handle scrolling of empty lines
                if (text[l] == "")
                {
                    Font::GlyphInfo glyph;

                    if (font->getRect( ' ', glyph) && glyph.rect.h > 0)
                    {
                        rect.h   = static_cast<int>( glyph.rect.h * scale * scaleY_ );

                        // Check if the glyph falls (partially) within the box at the front end
                        if ((position + font->getHeight( ) * scale * scaleY_ > currentPosition_) &&
                            (position < currentPosition_))
                        {
                            yAdvance  -= rect.h - static_cast<int>( font->getHeight( ) * scale * scaleX_ + position - currentPosition_ );
                        }
                    }
                }

                if ( position + font->getHeight( ) * scale * scaleY_ > currentPosition_ )
                {
                    rect.y += yAdvance;
                }
                position += font->getHeight( ) * scale * scaleY_;

            }

            // Reset scrolling position when we're done
            if (currentPosition_ > text.size( ) * font->getHeight( ) * scale * scaleX_)
            {
                waitStartTime_   = startTime_;
                waitEndTime_     = endTime_;
                currentPosition_ = -startPosition_ * scaleY_;
            }

        }
    }
}

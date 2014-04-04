//
//  Font.cpp
//  MoFlow
//
//  Created by Scott Downie on 26/10/2010.
//  Copyright (c) 2010 Tag Games. All rights reserved.
//

#include <ChilliSource/Rendering/Font/Font.h>

#include <ChilliSource/Core/String/UTF8String.h>
#include <ChilliSource/Core/Math/MathUtils.h>

#include <ChilliSource/Rendering/Material/Material.h>
#include <ChilliSource/Rendering/Texture/Texture.h>
#include <ChilliSource/Rendering/Sprite/SpriteComponent.h>

namespace ChilliSource
{
	namespace Rendering
	{
        
        f32 Font::mfGlobalKerningOffset = 0.0f;
        
        CS_DEFINE_NAMEDTYPE(Font);
        
		//---------------------------------------------------------------------
		/// Is A
		///
		/// @return Whether this object is of given type
		//---------------------------------------------------------------------
		bool Font::IsA(Core::InterfaceIDType inInterfaceID) const
		{
			return inInterfaceID == Font::InterfaceID;
		}
		//-------------------------------------------
		/// Set Character Set
		///
		/// The list of characters that this font
		/// can display
		/// @param An array of the characters
		//-------------------------------------------
		void Font::SetCharacterSet(const CharacterSet &inSet)
		{
			mCharacterSet = inSet;
		}
		//-------------------------------------------
		/// Set Texture
		///
		/// @param Texture containing the font 
		//-------------------------------------------
		void Font::SetTexture(const TextureSPtr& inpTex)
		{
			mpTexture = inpTex;
			mpTexture->SetFilter(Texture::Filter::k_linear, Texture::Filter::k_linear);
		}
		//-------------------------------------------
		/// Get Texture
		///
		/// @return Font texture 
		//-------------------------------------------
		const TextureSPtr& Font::GetTexture() const
		{
			return mpTexture;
		}
		//-------------------------------------------
		/// Get Attributes
		///
		/// @return Attributes - Spacing etc
		//-------------------------------------------
		const FontAttributes& Font::GetAttributes() const
		{
			return mAttributes;
		}
		//-------------------------------------------
		/// Get Mode Character Height
		///
		/// @return Mode height
		//-------------------------------------------
		f32 Font::GetLineHeight() const
		{
			return mfLineHeight;
        }
        //-------------------------------------------
        /// Supports Kerning
        ///
        /// @return True is kerning supported, false otherwise
        //-------------------------------------------
        const bool Font::SupportsKerning() const
        {
            return (!maFirstLookup.empty() && !maPairs.empty());
        }
		//-------------------------------------------
		/// Set Character Data
		///
		/// Each character is a sprite on a tpage
		/// therefore we need the UV offsets
		///
		/// @param Sprite data containing UV's etc
		//-------------------------------------------
		void Font::SetSpriteSheet(const SpriteSheetSPtr& inpData)
		{
			mpCharacterData = inpData;
			mpTexture = inpData->GetTexture();
			
			mMapCharToCharInfo.clear();
			
            const f32 fSpriteSheetWidth = mpCharacterData->GetSpriteSheetWidth();
            const f32 fSpriteSheetHeight = mpCharacterData->GetSpriteSheetHeight();

			for (u32 nChar = 0; nChar < mCharacterSet.length(); nChar++) 
            {
				CharacterInfo sCI;
				
				SpriteSheet::Frame sFrame = mpCharacterData->GetSpriteFrameByID(nChar);
				
				//Get the UV co-ordinates of the character on the font tpage
				mpCharacterData->GetUVsForFrame(nChar, sCI.sUVs);
								
				sCI.sUVs.vOrigin.x = (f32)(sFrame.U - 0.5f) / fSpriteSheetWidth;
				sCI.sUVs.vOrigin.y = (f32)(sFrame.V - 0.5f) / fSpriteSheetHeight;
				sCI.sUVs.vSize.x = (f32)(sFrame.Width + 1.0f) / fSpriteSheetWidth;
				sCI.sUVs.vSize.y = (f32)(sFrame.Height + 1.0f) / fSpriteSheetHeight;
				
				sCI.vSize.x = sFrame.Width;
                sCI.vSize.y = sFrame.Height;
                
				sCI.vOffset.x = sCI.vSize.x * 0.5f;
				sCI.vOffset.y = (sFrame.OffsetY);
                
                mfLineHeight = std::max((f32)sFrame.OriginalHeight, mfLineHeight);

                Core::UTF8String::Char Char = mCharacterSet[nChar];
				mMapCharToCharInfo.insert(std::make_pair(Char, sCI));
			}
		
			
			//Just assign the width of a space based on the similar space character in the 
			//font. This means it will scale relative to the font
			CharacterInfo sInfo;
			if(GetInfoForCharacter(kSimilarSpaceCharacter, sInfo) == CharacterResult::k_ok)
			{
				mAttributes.SpaceSpacing = sInfo.vSize.x;
				mAttributes.TabSpacing = sInfo.vSize.x * kudwSpacesPerTab;
			}
			else
			{
				CS_LOG_ERROR("Invalid space character chosen in font please include a '-' in the font");
			}
            
        }
		//-------------------------------------------
		/// Get Frame By Character
		///
		/// Get the tpage data for the given char
		/// @param Wide char
		/// @param Frame to be filled with data 
		/// @return Success or invisible chars
		//-------------------------------------------
		CharacterResult Font::GetInfoForCharacter(Core::UTF8String::Char inChar, CharacterInfo& outInfo) const
		{
			switch(inChar)
            {
                case kSpaceCharacter:
                    return CharacterResult::k_space;
                case kTabCharacter:
                    return CharacterResult::k_tab;
                case kReturnCharacter:
                    return CharacterResult::k_return;
            }

            if(Core::UTF8String::IsNonBreakingSpace(inChar) == true)
            {
                return CharacterResult::k_nbsp;
            }
			
			CharToCharInfoMap::const_iterator pCharEntry = mMapCharToCharInfo.find(inChar);
			
			if (pCharEntry != mMapCharToCharInfo.end()) 
            {
				outInfo = pCharEntry->second;
				
				return CharacterResult::k_ok;
			}
			
			return CharacterResult::k_invalid;
		}
        //-------------------------------------------
        /// Get Kerning Between Characters
        //-------------------------------------------
        f32 Font::GetKerningBetweenCharacters(Core::UTF8String::Char inChar1, Core::UTF8String::Char inChar2) const
        {
            const KernLookup* pLookup = &(*std::lower_bound(maFirstLookup.begin(), maFirstLookup.end(), KernLookup(inChar1, 0)));
			if(nullptr == pLookup || pLookup->wCharacter != inChar1)
            {
				return mfGlobalKerningOffset;
            }
			
            u32 udwStart = pLookup->uwStart;
			u32 udwEnd = udwStart + pLookup->uwLength;
			const KernPair* pPair = &(*std::lower_bound(maPairs.begin() + udwStart, maPairs.begin() + udwEnd, KernPair(inChar2, 0.0f)));
			if(nullptr == pPair || pPair->wCharacter != inChar2)
            {
				return mfGlobalKerningOffset;
			}

            return pPair->fSpacing + mfGlobalKerningOffset;
        }
        //-------------------------------------------
        /// Set Kerning Info
        //-------------------------------------------
        void Font::SetKerningInfo(const std::vector<KernLookup>& inaFirstReg, const std::vector<KernPair>& inaPairs)
        {
            maFirstLookup = inaFirstReg;
            maPairs = inaPairs;
        }
        //-------------------------------------------
        /// Set Global Kerning Offset
        //-------------------------------------------
        void Font::SetGlobalKerningOffset(f32 infOffset)
        {
            mfGlobalKerningOffset = infOffset;
        }
	}
}

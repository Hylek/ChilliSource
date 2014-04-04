//
//  Font.h
//  MoFlow
//
//  Created by Scott Downie on 26/10/2010.
//  Copyright (c) 2010 Tag Games. All rights reserved.
//

#ifndef _MO_FLO_RENDERING_FONT_H_
#define _MO_FLO_RENDERING_FONT_H_

#include <ChilliSource/ChilliSource.h>
#include <ChilliSource/Rendering/Sprite/SpriteSheet.h>
#include <ChilliSource/Core/Math/Geometry/Shapes.h>
#include <ChilliSource/Core/Resource/ResourceOld.h>
#include <ChilliSource/Core/String/UTF8String.h>

namespace ChilliSource
{
	namespace Rendering
	{
		//---Font Types
		typedef u32 FontAlignment;
		typedef f32 FontSpacing;
		
		typedef Core::UTF8String CharacterSet;
		
		const Core::UTF8String::Char kSimilarSpaceCharacter = '-';
		const Core::UTF8String::Char kReturnCharacter = '\n';
		const Core::UTF8String::Char kTabCharacter = '\t';
        const Core::UTF8String::Char kSpaceCharacter = ' ';
        
        const u32 kudwSpacesPerTab = 5;
		
		//---Font Attributes
		struct FontAttributes
		{
			FontAttributes() : TabSpacing(10.0f), SpaceSpacing(6.0f){}
			
			FontSpacing TabSpacing;
            FontSpacing SpaceSpacing;
		};
		
		//---Font Values
		enum class CharacterResult
		{
            k_space,
            k_tab,
            k_return,
            k_invalid,
            k_ok
		};
		
		//Details how a particular character must be drawn
		struct PlacedCharacter
		{
			Core::Rectangle sUVs;
			Core::Vector2 vSize;
			Core::Vector2 vPosition;
		};
		
		typedef std::vector<PlacedCharacter> CharacterList;
        
		class Font : public Core::ResourceOld
		{
		public:

			//Internal cache of what a character should be built of
			struct CharacterInfo
			{
				Core::Rectangle sUVs;
                Core::Vector2 vSize;
                Core::Vector2 vOffset;
			};
            
            struct KernLookup
			{
                //---------------------------------------------------------------------
                /// Constructor
                //---------------------------------------------------------------------
				KernLookup(s16 inwCharacter, u16 inuwStart)
				: wCharacter(inwCharacter)
				, uwStart(inuwStart)
				, uwLength(0)
				{
				}
				//---------------------------------------------------------------------
                /// Less then operator overload
                //---------------------------------------------------------------------
				bool operator < (const KernLookup& inB) const
				{
                    return wCharacter < inB.wCharacter;
                }
                    
                s16 wCharacter;
                u16 uwStart;
                u16 uwLength;
            };
                    
            struct KernPair
            {
                //---------------------------------------------------------------------
                /// Constructor
                //---------------------------------------------------------------------
                KernPair(s16 inwCharacter, f32 infSpacing)
                : wCharacter(inwCharacter)
                , fSpacing(infSpacing)
                {
                }
                //---------------------------------------------------------------------
                /// Less then operator overload
                //---------------------------------------------------------------------
                bool operator < (const KernPair& inB) const
                {
                    return wCharacter < inB.wCharacter;
                }
                
                f32 fSpacing;
                s16 wCharacter;				
            };
			
			virtual ~Font(){}
			CS_DECLARE_NAMEDTYPE(Font);
			//---------------------------------------------------------------------
			/// Is A
			///
			/// @return Whether this object is of given type
			//---------------------------------------------------------------------
			bool IsA(Core::InterfaceIDType inInterfaceID) const override;
            //-------------------------------------------
            /// Set Texture
            ///
            /// @param Texture containing the font
            //-------------------------------------------
            void SetTexture(const TextureSPtr& inpTex);
			//-------------------------------------------
			/// Get Texture
			///
			/// @return Font texture 
			//-------------------------------------------
			const TextureSPtr& GetTexture() const;
			//-------------------------------------------
			/// Set Character Data
			///
			/// Each character is a sprite on a tpage
			/// therefore we need the UV offsets
			///
			/// @param Sprite data containing UV's etc
			//-------------------------------------------
			void SetSpriteSheet(const SpriteSheetSPtr& inpData);
			//-------------------------------------------
			/// Get Mode Character Height
			///
			/// @return Mode height
			//-------------------------------------------
			f32 GetLineHeight() const;
            //-------------------------------------------
            /// Supports Kerning
            ///
            /// @return True is kerning supported, false otherwise
            //-------------------------------------------
            const bool SupportsKerning() const;
            //-------------------------------------------
			/// Get Info For Character
			///
			/// Get the info struct-a-majig for this character
			/// @param Character
			/// @param info struct to be filled with data for the character
			/// @return Success or invisible chars
			//-------------------------------------------
			CharacterResult GetInfoForCharacter(Core::UTF8String::Char inChar, CharacterInfo& outInfo) const;
			//-------------------------------------------
			/// Get Attributes
			///
			/// @return Attributes - Spacing etc
			//-------------------------------------------
			const FontAttributes& GetAttributes() const;
			//-------------------------------------------
			/// Set Character Set
			///
			/// The list of characters that this font
			/// can display
			/// @param An array of the characters
			//-------------------------------------------
			void SetCharacterSet(const CharacterSet &inSet);
            //-------------------------------------------
            /// Get Kerning Between Characters
            ///
            /// Returns the amount that the glyph represented
            /// by inChar2 can be moved towards inChar1 without
            /// overlapping.
            /// This data is generated by the SpriteTool
            ///
            /// @param First character to test
            /// @param Second character to test
            /// @return Spacing between characters
            //-------------------------------------------
            f32 GetKerningBetweenCharacters(Core::UTF8String::Char inChar1, Core::UTF8String::Char inChar2) const;
            //-------------------------------------------
            /// Set Global Kerning Offset
            ///
            /// Sets a value that will be used to offset all kerning values
            ///
            /// @param Offset
            //-------------------------------------------
            static void SetGlobalKerningOffset(f32 infOffset);
            //-------------------------------------------
            /// Set Kerning Info
            ///
            /// Sets kerning lookup and pair info
            ///
            /// @param Kerning lookup array
            /// @param Kerning pair array
            //-------------------------------------------
            void SetKerningInfo(const std::vector<KernLookup>& inaFirstReg, const std::vector<KernPair>& inaPairs);
		
        private:
            friend class FontManager;
                    
			//Only the font manager can create this
            Font() : mfLineHeight(0.0f)
            {
                maFirstLookup.clear();
                maPairs.clear();
            }
            
			typedef std::unordered_map<Core::UTF8String::Char, CharacterInfo> CharToCharInfoMap;
			
			CharToCharInfoMap mMapCharToCharInfo;
			
			//Holds the size, spacing, kerning etc
			FontAttributes mAttributes;
			
			//Holds all the characters that the font supports
			CharacterSet mCharacterSet;
			
            // Kerning lookup
            std::vector<KernLookup> maFirstLookup;
                    
            // Kerning pairs
            std::vector<KernPair> maPairs;
                    
			//The font bitmap
			TextureSPtr mpTexture;
			
			//The location and width etc of each character on the tpage
			SpriteSheetSPtr mpCharacterData;
            
            f32 mfLineHeight;
            
            static f32 mfGlobalKerningOffset;
		};
	}
}

#endif

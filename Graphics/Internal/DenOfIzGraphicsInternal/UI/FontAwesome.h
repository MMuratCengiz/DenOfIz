/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include <cstdint>

namespace DenOfIz::FontAwesome
{
    constexpr auto Play         = "\uf04b";
    constexpr auto Pause        = "\uf04c";
    constexpr auto Stop         = "\uf04d";
    constexpr auto StepForward  = "\uf051";
    constexpr auto StepBackward = "\uf04a";
    constexpr auto Forward      = "\uf04e";
    constexpr auto Backward     = "\uf04a";
    constexpr auto FastForward  = "\uf050";
    constexpr auto FastBackward = "\uf049";
    constexpr auto Eject        = "\uf052";

    constexpr auto Home       = "\uf015";
    constexpr auto File       = "\uf15b";
    constexpr auto FileAlt    = "\uf15c";
    constexpr auto Folder     = "\uf07b";
    constexpr auto FolderOpen = "\uf07c";
    constexpr auto Save       = "\uf0c7";
    constexpr auto Download   = "\uf019";
    constexpr auto Upload     = "\uf093";

    constexpr auto Edit     = "\uf044";
    constexpr auto Cut      = "\uf0c4";
    constexpr auto Copy     = "\uf0c5";
    constexpr auto Paste    = "\uf0ea";
    constexpr auto Trash    = "\uf1f8";
    constexpr auto TrashAlt = "\uf2ed";

    constexpr auto Plus        = "\uf067";
    constexpr auto Minus       = "\uf068";
    constexpr auto Times       = "\uf00d";
    constexpr auto Check       = "\uf00c";
    constexpr auto CheckCircle = "\uf058";
    constexpr auto CheckSquare = "\uf14a";

    constexpr auto Search      = "\uf002";
    constexpr auto SearchPlus  = "\uf00e";
    constexpr auto SearchMinus = "\uf010";

    constexpr auto User       = "\uf007";
    constexpr auto Users      = "\uf0c0";
    constexpr auto UserCircle = "\uf2bd";
    constexpr auto UserCog    = "\uf53d";

    constexpr auto Cog    = "\uf013";
    constexpr auto Cogs   = "\uf085";
    constexpr auto Wrench = "\uf0ad";
    constexpr auto Tools  = "\uf7d9";

    constexpr auto Eye      = "\uf06e";
    constexpr auto EyeSlash = "\uf070";

    constexpr auto Lock     = "\uf023";
    constexpr auto LockOpen = "\uf3c1";
    constexpr auto Unlock   = "\uf09c";

    constexpr auto ArrowUp    = "\uf062";
    constexpr auto ArrowDown  = "\uf063";
    constexpr auto ArrowLeft  = "\uf060";
    constexpr auto ArrowRight = "\uf061";

    constexpr auto ChevronUp    = "\uf077";
    constexpr auto ChevronDown  = "\uf078";
    constexpr auto ChevronLeft  = "\uf053";
    constexpr auto ChevronRight = "\uf054";

    constexpr auto CaretUp    = "\uf0d7";
    constexpr auto CaretDown  = "\uf0d7";
    constexpr auto CaretLeft  = "\uf0d9";
    constexpr auto CaretRight = "\uf0da";

    constexpr auto AngleUp    = "\uf066";
    constexpr auto AngleDown  = "\uf067";
    constexpr auto AngleLeft  = "\uf104";
    constexpr auto AngleRight = "\uf105";

    constexpr auto Circle = "\uf111";
    constexpr auto Square = "\uf0c8";
    constexpr auto Cube   = "\uf1f3";
    constexpr auto Cubes  = "\uf1fb";

    constexpr auto Info                = "\uf129";
    constexpr auto InfoCircle          = "\uf05a";
    constexpr auto Question            = "\uf128";
    constexpr auto QuestionCircle      = "\uf059";
    constexpr auto Exclamation         = "\uf12a";
    constexpr auto ExclamationCircle   = "\uf05c";
    constexpr auto ExclamationTriangle = "\uf071";

    constexpr auto Bars    = "\uf0c9";
    constexpr auto List    = "\uf03a";
    constexpr auto ListAlt = "\uf022";
    constexpr auto ListOl  = "\uf0cb";
    constexpr auto ListUl  = "\uf0ca";

    constexpr auto Table   = "\uf0ce";
    constexpr auto Columns = "\uf0df";
    constexpr auto ThLarge = "\uf009";
    constexpr auto Th      = "\uf00a";
    constexpr auto ThList  = "\uf00b";

    constexpr auto Image  = "\uf03e";
    constexpr auto Images = "\uf302";
    constexpr auto Camera = "\uf030";
    constexpr auto Video  = "\uf03d";

    constexpr auto Sun      = "\uf185";
    constexpr auto Moon     = "\uf186";
    constexpr auto Star     = "\uf005";
    constexpr auto StarHalf = "\uf089";

    constexpr auto LightbulbOn  = "\uf0eb";
    constexpr auto LightbulbOff = "\uf0eb";

    constexpr auto Heart       = "\uf004";
    constexpr auto HeartBroken = "\uf7a9";

    constexpr auto Gears    = "\uf085";
    constexpr auto Sliders  = "\uf1de";
    constexpr auto SlidersH = "\uf1de";

    constexpr auto Paint      = "\uf07c";
    constexpr auto PaintBrush = "\uf1fc";
    constexpr auto Palette    = "\uf53f";

    constexpr auto Compress = "\uf066";
    constexpr auto Expand   = "\uf065";

    constexpr auto CloudDownload = "\uf0c1";
    constexpr auto CloudUpload   = "\uf0c2";

    constexpr auto Database  = "\uf1c0";
    constexpr auto Server    = "\uf233";
    constexpr auto HardDrive = "\uf0a0";

    constexpr auto Code     = "\uf121";
    constexpr auto Terminal = "\uf120";
    constexpr auto Bug      = "\uf188";

    constexpr auto Book     = "\uf02e";
    constexpr auto BookOpen = "\uf538";
    constexpr auto Bookmark = "\uf02e";

    constexpr auto Calendar    = "\uf133";
    constexpr auto CalendarAlt = "\uf133";
    constexpr auto Clock       = "\uf017";

    constexpr auto MapMarker = "\uf041";
    constexpr auto MapPin    = "\uf276";
    constexpr auto Globe     = "\uf0ac";

    constexpr auto Link   = "\uf0c1";
    constexpr auto Unlink = "\uf127";
    constexpr auto Chain  = "\uf0c1";

    constexpr auto Wifi      = "\uf1eb";
    constexpr auto Bluetooth = "\uf293";
    constexpr auto Signal    = "\uf012";

    constexpr auto Bell      = "\uf0f3";
    constexpr auto BellSlash = "\uf1f6";

    constexpr auto Volume     = "\uf027";
    constexpr auto VolumeUp   = "\uf028";
    constexpr auto VolumeDown = "\uf027";
    constexpr auto VolumeOff  = "\uf026";
    constexpr auto VolumeMute = "\uf6a9";

    constexpr auto Microphone      = "\uf130";
    constexpr auto MicrophoneSlash = "\uf131";

    constexpr auto Battery      = "\uf240";
    constexpr auto BatteryFull  = "\uf240";
    constexpr auto BatteryHalf  = "\uf242";
    constexpr auto BatteryEmpty = "\uf244";

    constexpr auto Plug  = "\uf1e6";
    constexpr auto Power = "\uf011";

    constexpr auto Refresh = "\uf021";
    constexpr auto Sync    = "\uf021";
    constexpr auto Redo    = "\uf01e";
    constexpr auto Undo    = "\uf0e2";

    constexpr auto Random  = "\uf074";
    constexpr auto Shuffle = "\uf074";

    constexpr auto Sort     = "\uf0dc";
    constexpr auto SortUp   = "\uf0de";
    constexpr auto SortDown = "\uf0dd";

    constexpr auto DenOfIz_Filter = "\uf0b0";

    constexpr auto Crop     = "\uf125";
    constexpr auto Scissors = "\uf0c4";

    constexpr auto Print = "\uf02f";
    constexpr auto Fax   = "\uf1ec";

    constexpr auto Envelope     = "\uf0e0";
    constexpr auto EnvelopeOpen = "\uf2b6";

    constexpr auto Comment    = "\uf075";
    constexpr auto Comments   = "\uf086";
    constexpr auto CommentAlt = "\uf277";

    constexpr auto Thumbsup   = "\uf164";
    constexpr auto Thumbsdown = "\uf165";

    constexpr auto Flag          = "\uf024";
    constexpr auto FlagCheckered = "\uf11e";

    constexpr auto Tag  = "\uf02b";
    constexpr auto Tags = "\uf02c";

    constexpr auto Trophy = "\uf091";
    constexpr auto Award  = "\uf559";
    constexpr auto Medal  = "\uf5a2";

    constexpr auto Gift    = "\uf06b";
    constexpr auto Gamepad = "\uf11b";

    constexpr auto Rocket = "\uf135";
    constexpr auto Plane  = "\uf072";
    constexpr auto Car    = "\uf1f9";
    constexpr auto Truck  = "\uf0f9";

    constexpr auto Anchor = "\uf13d";
    constexpr auto Ship   = "\uf231";

    constexpr auto Snowflake = "\uf2dc";
    constexpr auto Fire      = "\uf06d";

    constexpr auto Spinner     = "\uf110";
    constexpr auto CircleNotch = "\uf1ce";

    constexpr auto Sitemap = "\uf0e8";
    constexpr auto Tree    = "\uf1fb";

    constexpr auto Layer  = "\uf5fd";
    constexpr auto Layers = "\uf5fd";

    constexpr auto Component = "\uf1fb";

    constexpr auto Hammer      = "\uf6c7";
    constexpr auto Screwdriver = "\uf6ca";

    constexpr auto Plugin = "\uf1ea";
    constexpr auto Puzzle = "\uf12e";

    constexpr auto Magic = "\uf0d0";
    constexpr auto Wand  = "\uf0d0";
} // namespace DenOfIz::FontAwesome

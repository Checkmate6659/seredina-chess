#include "eval.hpp"
#include "chess.hpp"

//PSQT: pawn, knight, bishop, rook, queen, king
//Layout: A8 B8 ... H8 A7 ... H1: WARNING: for white it is flipped, for Black it isn't
constexpr ValPair psqt[] = {
    S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(227, 556), S(292, 535), S(232, 524), S(291, 438), S(259, 437), S(244, 456), S(125, 542), S(72, 568), S(128, 293), S(153, 270), S(198, 201), S(203, 145), S(213, 137), S(270, 162), S(230, 231), S(159, 251), S(94, 243), S(139, 218), S(140, 183), S(146, 156), S(182, 154), S(168, 166), S(178, 200), S(132, 199), S(77, 213), S(123, 208), S(121, 178), S(151, 168), S(150, 167), S(136, 175), S(153, 193), S(114, 179), S(73, 201), S(116, 205), S(116, 174), S(117, 185), S(141, 183), S(126, 180), S(179, 188), S(125, 171), S(70, 208), S(115, 210), S(104, 187), S(87, 187), S(118, 203), S(152, 186), S(191, 188), S(110, 171), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), 
    S(239, 456), S(315, 542), S(417, 573), S(477, 549), S(538, 549), S(377, 531), S(360, 534), S(335, 414), S(467, 551), S(509, 577), S(578, 575), S(582, 582), S(553, 568), S(667, 545), S(512, 565), S(533, 516), S(501, 565), S(572, 585), S(593, 617), S(614, 617), S(689, 587), S(700, 578), S(611, 572), S(564, 541), S(494, 585), S(518, 620), S(557, 639), S(596, 641), S(558, 645), S(611, 631), S(532, 618), S(554, 571), S(470, 585), S(498, 603), S(519, 641), S(520, 644), S(537, 647), S(527, 631), S(528, 607), S(487, 571), S(437, 557), S(476, 591), S(496, 606), S(506, 631), S(525, 627), S(504, 598), S(515, 579), S(461, 563), S(418, 542), S(436, 570), S(464, 588), S(482, 591), S(484, 589), S(495, 580), S(470, 554), S(464, 553), S(342, 542), S(431, 503), S(412, 562), S(438, 567), S(446, 564), S(465, 548), S(438, 510), S(401, 523), 
    S(506, 605), S(492, 618), S(457, 622), S(418, 635), S(432, 628), S(467, 612), S(529, 605), S(486, 593), S(542, 593), S(594, 613), S(570, 622), S(531, 622), S(595, 609), S(611, 607), S(588, 619), S(532, 592), S(550, 630), S(600, 617), S(610, 631), S(637, 614), S(620, 621), S(679, 627), S(627, 620), S(600, 619), S(543, 623), S(559, 649), S(605, 637), S(625, 658), S(617, 650), S(612, 641), S(562, 643), S(539, 626), S(530, 616), S(555, 639), S(557, 656), S(595, 654), S(592, 650), S(560, 646), S(553, 634), S(543, 597), S(544, 609), S(558, 629), S(557, 644), S(560, 644), S(561, 650), S(557, 641), S(560, 613), S(566, 594), S(547, 603), S(550, 600), S(569, 601), S(532, 624), S(543, 627), S(569, 607), S(577, 609), S(555, 567), S(509, 578), S(545, 609), S(514, 571), S(507, 611), S(514, 604), S(507, 599), S(545, 585), S(532, 559), 
    S(750, 1095), S(746, 1101), S(758, 1113), S(767, 1106), S(796, 1091), S(785, 1091), S(761, 1093), S(797, 1081), S(724, 1100), S(724, 1115), S(757, 1118), S(793, 1103), S(766, 1101), S(818, 1081), S(793, 1079), S(843, 1060), S(682, 1103), S(724, 1101), S(728, 1101), S(732, 1099), S(781, 1075), S(786, 1067), S(857, 1052), S(800, 1051), S(653, 1104), S(674, 1101), S(682, 1114), S(704, 1105), S(710, 1085), S(710, 1078), S(722, 1070), S(717, 1069), S(619, 1095), S(630, 1098), S(647, 1102), S(669, 1095), S(666, 1090), S(637, 1089), S(680, 1067), S(657, 1066), S(606, 1083), S(628, 1082), S(639, 1081), S(637, 1089), S(650, 1077), S(642, 1069), S(697, 1037), S(658, 1041), S(601, 1075), S(626, 1077), S(649, 1079), S(644, 1082), S(652, 1068), S(656, 1061), S(681, 1046), S(615, 1066), S(628, 1062), S(636, 1080), S(651, 1093), S(659, 1089), S(667, 1076), S(647, 1067), S(666, 1065), S(631, 1043), 
    S(1439, 1972), S(1437, 2013), S(1502, 2018), S(1539, 2006), S(1566, 1990), S(1592, 1970), S(1583, 1921), S(1486, 1995), S(1488, 1941), S(1452, 2012), S(1473, 2049), S(1457, 2077), S(1463, 2104), S(1547, 2047), S(1510, 2040), S(1586, 1978), S(1488, 1959), S(1486, 1982), S(1492, 2038), S(1505, 2054), S(1526, 2070), S(1600, 2034), S(1595, 1987), S(1584, 1970), S(1454, 1989), S(1461, 2015), S(1473, 2032), S(1473, 2066), S(1476, 2093), S(1501, 2060), S(1492, 2060), S(1495, 2031), S(1458, 1958), S(1455, 2014), S(1458, 2014), S(1465, 2065), S(1468, 2051), S(1465, 2038), S(1482, 2010), S(1488, 1987), S(1454, 1941), S(1471, 1952), S(1461, 2001), S(1458, 1999), S(1462, 2008), S(1472, 1996), S(1495, 1957), S(1485, 1938), S(1453, 1918), S(1464, 1926), S(1480, 1923), S(1477, 1944), S(1475, 1949), S(1493, 1908), S(1500, 1858), S(1519, 1814), S(1453, 1911), S(1438, 1921), S(1446, 1933), S(1468, 1906), S(1455, 1931), S(1432, 1923), S(1470, 1871), S(1454, 1873), 
    S(55, -165), S(148, -103), S(142, -74), S(-25, -10), S(-58, -13), S(3, -4), S(130, -32), S(186, -153), S(-21, -29), S(-24, 37), S(-71, 48), S(81, 22), S(15, 48), S(7, 75), S(13, 63), S(-26, 20), S(-101, 9), S(44, 37), S(-35, 63), S(-82, 77), S(-15, 79), S(127, 72), S(79, 71), S(-16, 27), S(-81, -8), S(-104, 50), S(-93, 70), S(-179, 93), S(-152, 91), S(-106, 88), S(-98, 72), S(-165, 33), S(-86, -29), S(-68, 16), S(-143, 64), S(-177, 85), S(-178, 85), S(-126, 65), S(-126, 42), S(-172, 13), S(4, -45), S(26, -8), S(-67, 29), S(-91, 51), S(-85, 51), S(-83, 37), S(-2, 2), S(-32, -18), S(146, -80), S(73, -32), S(48, -10), S(-20, 10), S(-19, 15), S(15, -3), S(97, -35), S(115, -70), S(136, -141), S(174, -106), S(123, -72), S(-47, -36), S(60, -84), S(0, -44), S(137, -92), S(145, -141)
};
//basic tempo evaluation
constexpr ValPair tempo = S(26, 36);
//passed pawn bonus by rank
constexpr ValPair passer_coefs[] = {S(0, 0), S(-14, -15), S(-29, -3), S(-26, 28), S(17, 69), S(13, 163), S(0, 0), S(0, 0)};

int gamephaseInc[12] = {0,1,1,2,4,0,0,1,1,2,4,0};
int mg_table[12][64];
int eg_table[12][64];

void init_tables()
{
    for (int pc = (int)Piece::WHITEPAWN; pc <= (int)Piece::WHITEKING; pc++) {
        for (int sq = 0; sq < 64; sq++) {
            mg_table[pc]  [sq] = MG(psqt[pc * 64 + (sq ^ 56)]);
            eg_table[pc]  [sq] = EG(psqt[pc * 64 + (sq ^ 56)]);
            mg_table[pc+6][sq] = MG(psqt[pc * 64 + sq]);
            eg_table[pc+6][sq] = EG(psqt[pc * 64 + sq]);
        }
    }
}

Value eval(Board board)
{
    Value mg = 0, eg = 0;
    uint8_t mgPhase = 0;

    //evaluate each piece (inefficient)
    for (uint8_t sq = 0; sq < 64; ++sq) {
        uint8_t pc = (int)board.at<Piece>(sq);
        if (pc != (int)Piece::NONE) {
            int8_t perspective = (pc >= 6) ? -1 : 1; //-1 if black, 1 if white
            mg += mg_table[pc][sq] * perspective;
            eg += eg_table[pc][sq] * perspective;
            mgPhase += gamephaseInc[pc];
        }
    }

    //create pawn bitboards (NOTE: square A1 is gonna be 0!!! no flipping required tho)
    Bitboard wpawns = board.pieces(PieceType::PAWN, Color::WHITE);
    Bitboard bpawns = board.pieces(PieceType::PAWN, Color::BLACK);
    //squares on the sides of pawns
    Bitboard wsides = ((wpawns << 1) & 0x7f7f7f7f7f7f7f7f) | ((wpawns >> 1) & 0xfefefefefefefefe);
    Bitboard bsides = ((bpawns << 1) & 0x7f7f7f7f7f7f7f7f) | ((bpawns >> 1) & 0xfefefefefefefefe);
    //White and Black frontspans | atk frontspans (hardcoded fill alg)
    Bitboard wspans = (wpawns | wsides) << 8;
    wspans |= wspans << 8;
    wspans |= wspans << 16;
    wspans |= wspans << 32;
    Bitboard bspans = (bpawns | bsides) >> 8;
    bspans |= bspans >> 8;
    bspans |= bspans >> 16;
    bspans |= bspans >> 32;
    //passer bitboards! finally!
    Bitboard wpassed = wpawns & ~bspans;
    Bitboard bpassed = bpawns & ~wspans;
    // std::cout << wpassed << std::endl; //DEBUG (NOTE: the bitboard gets *actually* printed!)

    //add that stuff up! (careful about the flip!)
    //tf is wrong? engine plays worse
    for (int i = 1; i < 6; i++)
    {
        //count passer difference
        int passer_count = (wpassed & (0xFFULL << (i*8))).count() //not flipped for white
                    - (bpassed & (0xFFULL << ((7-i)*8))).count(); //flipped for black here!
        // std::cout << passer_count << std::endl;
        //add to MG/EG scores
        mg += passer_count * MG(passer_coefs[i]);
        eg += passer_count * EG(passer_coefs[i]);
    }
    //test pos for bench: 8/3k4/5p2/5P2/4K1P1/8/8/8 w - - 0 1
    //no passers: 329cp at depth 10
    //1 to 6: 347cp
    //2 to 7, or 1 to 7: SAME
    //testing with another one: k7/8/8/8/8/8/6P1/K7 w - - 0 1
    //no passer eval: 224cp score
    //with passers (1 to 6): 209
    //with (0 to 8): SAME

    //it was Black to move: invert the evaluation
    if (board.sideToMove() == Color::BLACK)
    {
        mg *= -1;
        eg *= -1;
    }

    //tempo evaluation: always benefit stm, so do it after the flip
    mg += MG(tempo);
    eg += EG(tempo);

    //tapered eval
    if (mgPhase > 24) mgPhase = 24; /* in case of early promotion */
    uint8_t egPhase = 24 - mgPhase;
    return (mg * mgPhase + eg * egPhase) / 24;
}

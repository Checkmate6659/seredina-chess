#include "eval.hpp"
#include "bb_util.hpp"
#include "chess.hpp"

//PSQT: pawn, knight, bishop, rook, queen, king
//Layout: A8 B8 ... H8 A7 ... H1: WARNING: for white it is flipped, for Black it isn't
/*constexpr ValPair psqt[] = {S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(166, 656), S(339, 590), S(178, 566), S(257, 514), S(249, 536), S(356, 493), S(172, 594), S(-15, 656), S(154, 256), S(179, 230), S(192, 189), S(196, 139), S(286, 109), S(311, 130), S(231, 194), S(146, 221), S(127, 205), S(171, 178), S(159, 156), S(198, 120), S(196, 128), S(190, 137), S(183, 169), S(126, 172), S(115, 183), S(154, 171), S(166, 141), S(193, 128), S(201, 127), S(198, 127), S(190, 152), S(126, 148), S(122, 161), S(169, 162), S(156, 141), S(172, 137), S(190, 142), S(189, 135), S(240, 138), S(148, 135), S(124, 172), S(188, 158), S(154, 154), S(165, 145), S(163, 168), S(220, 141), S(257, 140), S(150, 128), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), 
S(347, 491), S(477, 467), S(557, 492), S(607, 450), S(828, 428), S(481, 473), S(630, 410), S(478, 419), S(525, 494), S(567, 507), S(819, 428), S(672, 498), S(694, 476), S(774, 441), S(683, 469), S(655, 439), S(567, 467), S(736, 451), S(667, 512), S(722, 506), S(819, 462), S(864, 463), S(776, 446), S(785, 421), S(654, 480), S(680, 492), S(655, 528), S(747, 519), S(715, 520), S(770, 500), S(688, 499), S(720, 472), S(654, 479), S(665, 477), S(655, 517), S(669, 532), S(686, 514), S(680, 518), S(700, 495), S(683, 478), S(637, 465), S(619, 491), S(639, 485), S(636, 514), S(669, 501), S(653, 477), S(686, 454), S(644, 473), S(643, 459), S(577, 487), S(621, 479), S(663, 471), S(654, 478), S(664, 454), S(631, 479), S(667, 446), S(470, 552), S(664, 451), S(570, 476), S(602, 491), S(650, 468), S(589, 483), S(666, 451), S(666, 458), 
S(668, 514), S(687, 503), S(460, 550), S(530, 539), S(611, 534), S(591, 528), S(644, 531), S(701, 503), S(676, 527), S(716, 522), S(648, 539), S(645, 515), S(731, 516), S(810, 501), S(731, 518), S(640, 532), S(671, 543), S(747, 510), S(755, 521), S(744, 514), S(734, 522), S(781, 529), S(767, 523), S(705, 547), S(696, 534), S(733, 528), S(706, 543), S(785, 530), S(750, 542), S(754, 537), S(727, 523), S(715, 544), S(719, 522), S(725, 524), S(729, 539), S(734, 554), S(756, 529), S(716, 529), S(714, 518), S(744, 522), S(735, 519), S(740, 523), S(725, 539), S(737, 534), S(728, 542), S(762, 514), S(727, 518), S(754, 520), S(768, 519), S(751, 501), S(747, 514), S(708, 527), S(726, 531), S(729, 514), S(781, 494), S(739, 498), S(692, 534), S(744, 537), S(732, 516), S(696, 534), S(710, 526), S(711, 522), S(652, 541), S(732, 526), 
S(952, 966), S(971, 958), S(918, 979), S(1011, 953), S(1006, 957), S(907, 976), S(920, 968), S(945, 957), S(949, 962), S(946, 969), S(1017, 953), S(1010, 955), S(1069, 917), S(1079, 923), S(956, 958), S(979, 951), S(884, 970), S(947, 963), S(958, 954), S(962, 958), S(937, 950), S(1021, 927), S(1075, 913), S(941, 942), S(863, 966), S(895, 958), S(909, 975), S(945, 948), S(952, 948), S(967, 947), S(927, 939), S(861, 970), S(831, 971), S(864, 966), S(895, 961), S(903, 951), S(919, 936), S(896, 939), S(937, 931), S(855, 943), S(831, 957), S(865, 953), S(884, 935), S(885, 940), S(909, 925), S(909, 922), S(898, 932), S(845, 936), S(839, 951), S(886, 936), S(883, 944), S(904, 944), S(919, 924), S(915, 924), S(897, 924), S(799, 961), S(885, 942), S(896, 951), S(917, 947), S(925, 942), S(918, 939), S(881, 945), S(855, 957), S(889, 913), 
S(1823, 1745), S(1785, 1850), S(1811, 1851), S(1810, 1841), S(2047, 1730), S(2031, 1732), S(1956, 1752), S(1946, 1801), S(1778, 1777), S(1730, 1821), S(1802, 1824), S(1800, 1859), S(1750, 1914), S(1953, 1783), S(1873, 1831), S(1954, 1777), S(1808, 1761), S(1788, 1786), S(1836, 1755), S(1803, 1875), S(1862, 1858), S(1941, 1820), S(1922, 1798), S(1951, 1794), S(1756, 1845), S(1787, 1797), S(1772, 1810), S(1771, 1841), S(1793, 1874), S(1830, 1845), S(1810, 1926), S(1829, 1900), S(1831, 1744), S(1758, 1844), S(1816, 1783), S(1792, 1832), S(1804, 1807), S(1811, 1822), S(1817, 1867), S(1823, 1863), S(1803, 1774), S(1845, 1700), S(1802, 1777), S(1830, 1752), S(1808, 1775), S(1831, 1790), S(1844, 1798), S(1843, 1825), S(1802, 1749), S(1820, 1726), S(1859, 1694), S(1843, 1713), S(1858, 1713), S(1846, 1708), S(1827, 1684), S(1857, 1723), S(1859, 1693), S(1852, 1691), S(1848, 1709), S(1861, 1691), S(1829, 1738), S(1776, 1741), S(1798, 1744), S(1767, 1729), 
S(-120, -163), S(229, -119), S(309, -105), S(193, -95), S(-197, -5), S(-110, 18), S(126, -50), S(100, -72), S(303, -106), S(116, 6), S(85, 16), S(243, -6), S(141, 8), S(100, 43), S(-63, 40), S(-253, 44), S(113, -31), S(152, 7), S(219, 20), S(54, 41), S(153, 29), S(277, 53), S(256, 34), S(-15, -6), S(11, -42), S(-11, 39), S(112, 46), S(-31, 98), S(-29, 92), S(-16, 79), S(29, 40), S(-156, 9), S(-101, -44), S(88, -26), S(-50, 63), S(-147, 107), S(-151, 111), S(-102, 72), S(-87, 29), S(-167, -10), S(1, -53), S(4, -11), S(-57, 44), S(-86, 68), S(-102, 74), S(-83, 54), S(-21, 11), S(-95, -16), S(-34, -59), S(5, -23), S(-32, 18), S(-107, 37), S(-79, 37), S(-60, 21), S(7, -16), S(-13, -50), S(-117, -100), S(36, -85), S(0, -45), S(-126, -5), S(-18, -47), S(-87, -18), S(23, -67), S(-18, -108)};

//basic tempo
constexpr int tempo = S(0, 0); //DISABLED

//passer bonus by rank
constexpr ValPair passer_coefs[] = {S(0, 0), S(-7, -18), S(-20, -5), S(-38, 29), S(4, 67), S(-3, 171), S(0, 0), S(0, 0)};

//for non-king: number of pawn-safe attacks (NOTE: pawn row is only 0s)
//for king: number of attacks as a queen
constexpr ValPair mobility[] = {S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), 
S(-69, -220), S(-18, -140), S(-7, -73), S(13, -27), S(44, -20), S(51, -4), S(68, 1), S(89, 2), S(108, -9), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), 
S(26, -127), S(-62, -155), S(-31, -86), S(4, -56), S(26, -43), S(45, -29), S(55, -16), S(67, -11), S(72, -3), S(84, -2), S(99, -8), S(137, -18), S(118, -6), S(177, -28), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), 
S(228, 76), S(-63, -98), S(-14, -154), S(-1, -115), S(11, -70), S(21, -56), S(30, -32), S(38, -28), S(56, -35), S(76, -34), S(99, -33), S(125, -30), S(150, -27), S(166, -31), S(187, -43), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), 
S(-100, -198), S(-75, -198), S(-50, -198), S(-32, -198), S(188, -588), S(212, -403), S(214, -259), S(225, -251), S(227, -255), S(238, -234), S(243, -202), S(246, -183), S(252, -155), S(261, -146), S(266, -131), S(273, -108), S(278, -114), S(275, -83), S(286, -93), S(280, -68), S(286, -49), S(305, -52), S(285, -32), S(330, -48), S(360, -60), S(488, -140), S(208, 34), S(554, -146), 
S(0, 0), S(0, 0), S(0, 0), S(71, 21), S(151, -43), S(117, 0), S(100, 1), S(81, 4), S(69, 5), S(59, 8), S(56, 5), S(37, 17), S(17, 19), S(1, 25), S(-24, 31), S(-36, 32), S(-54, 31), S(-73, 27), S(-91, 28), S(-105, 22), S(-94, 15), S(-88, 7), S(-138, 3), S(-98, -15), S(-157, -13), S(-98, -44), S(-210, -48), S(-63, -80)}; */
//NOTE: I may not have enough data for low-mobility queen (0 to 3 in the table)
//for the king (not safe mobility), minimum is 4 (surrounded in a corner)

constexpr ValPair psqt[] = {S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(158, 668), S(318, 603), S(168, 576), S(250, 520), S(220, 551), S(346, 503), S(150, 605), S(-37, 669), S(144, 268), S(176, 240), S(181, 202), S(191, 148), S(260, 121), S(301, 141), S(218, 206), S(133, 234), S(113, 218), S(156, 192), S(149, 169), S(182, 135), S(181, 143), S(176, 151), S(173, 181), S(113, 184), S(103, 194), S(144, 184), S(151, 156), S(179, 143), S(189, 144), S(183, 144), S(177, 167), S(113, 160), S(111, 173), S(156, 175), S(148, 155), S(158, 155), S(187, 157), S(175, 153), S(231, 152), S(137, 147), S(113, 183), S(175, 172), S(138, 171), S(167, 159), S(173, 176), S(229, 153), S(254, 153), S(138, 140), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), 
S(364, 490), S(475, 469), S(555, 492), S(549, 464), S(766, 442), S(432, 486), S(600, 418), S(472, 422), S(508, 503), S(558, 513), S(794, 437), S(636, 509), S(659, 485), S(709, 456), S(656, 478), S(616, 452), S(553, 477), S(726, 459), S(643, 524), S(692, 514), S(767, 475), S(811, 480), S(737, 456), S(750, 436), S(651, 487), S(659, 506), S(646, 537), S(707, 533), S(677, 532), S(733, 512), S(655, 512), S(695, 482), S(649, 485), S(661, 484), S(635, 529), S(648, 545), S(653, 528), S(646, 533), S(665, 507), S(665, 483), S(634, 472), S(613, 500), S(640, 490), S(628, 525), S(666, 510), S(643, 485), S(679, 462), S(638, 479), S(644, 464), S(577, 493), S(616, 487), S(672, 476), S(662, 484), S(679, 460), S(637, 484), S(663, 454), S(468, 555), S(662, 460), S(578, 482), S(606, 498), S(660, 474), S(613, 486), S(668, 456), S(667, 461), 
S(652, 527), S(650, 520), S(457, 558), S(497, 558), S(565, 551), S(557, 543), S(629, 543), S(704, 508), S(664, 536), S(702, 533), S(629, 550), S(615, 528), S(713, 526), S(748, 524), S(678, 537), S(600, 549), S(664, 551), S(745, 518), S(739, 531), S(726, 525), S(699, 537), S(755, 542), S(728, 538), S(684, 558), S(682, 546), S(708, 542), S(680, 558), S(759, 546), S(707, 562), S(712, 555), S(681, 544), S(679, 561), S(692, 539), S(706, 538), S(702, 554), S(710, 570), S(732, 545), S(693, 546), S(699, 530), S(701, 543), S(709, 535), S(722, 536), S(711, 550), S(716, 550), S(719, 554), S(752, 528), S(716, 531), S(727, 537), S(743, 532), S(736, 513), S(730, 529), S(710, 535), S(727, 541), S(740, 524), S(776, 509), S(723, 512), S(678, 545), S(718, 552), S(722, 533), S(702, 543), S(718, 534), S(716, 532), S(650, 556), S(716, 538), 
S(916, 990), S(937, 981), S(872, 1006), S(952, 985), S(947, 988), S(839, 1008), S(865, 997), S(889, 986), S(911, 989), S(911, 995), S(975, 982), S(968, 984), S(1012, 950), S(987, 964), S(884, 991), S(915, 982), S(851, 995), S(913, 989), S(901, 988), S(902, 993), S(865, 986), S(945, 967), S(1002, 947), S(854, 979), S(846, 987), S(866, 981), S(882, 1000), S(902, 979), S(910, 977), S(905, 983), S(869, 972), S(829, 996), S(817, 988), S(852, 983), S(873, 984), S(880, 975), S(895, 964), S(856, 969), S(889, 960), S(830, 967), S(817, 974), S(847, 973), S(867, 956), S(862, 965), S(892, 952), S(874, 955), S(865, 960), S(825, 956), S(823, 968), S(872, 952), S(868, 964), S(888, 967), S(907, 947), S(902, 949), S(877, 948), S(781, 979), S(869, 960), S(882, 969), S(906, 967), S(923, 958), S(927, 953), S(902, 953), S(843, 975), S(872, 933), 
S(1726, 1805), S(1697, 1896), S(1695, 1909), S(1691, 1887), S(1890, 1797), S(1892, 1792), S(1831, 1811), S(1847, 1848), S(1704, 1825), S(1655, 1870), S(1706, 1886), S(1712, 1902), S(1603, 1973), S(1792, 1845), S(1706, 1898), S(1841, 1829), S(1724, 1823), S(1713, 1836), S(1725, 1830), S(1671, 1943), S(1689, 1939), S(1760, 1890), S(1737, 1884), S(1774, 1855), S(1680, 1910), S(1705, 1871), S(1689, 1872), S(1655, 1922), S(1674, 1939), S(1708, 1878), S(1702, 1937), S(1723, 1908), S(1755, 1814), S(1682, 1905), S(1740, 1846), S(1714, 1892), S(1732, 1858), S(1723, 1858), S(1744, 1880), S(1736, 1877), S(1725, 1849), S(1770, 1768), S(1734, 1839), S(1759, 1811), S(1743, 1834), S(1754, 1842), S(1770, 1846), S(1767, 1868), S(1728, 1819), S(1751, 1787), S(1787, 1767), S(1783, 1770), S(1798, 1779), S(1796, 1770), S(1768, 1748), S(1791, 1769), S(1781, 1766), S(1779, 1754), S(1783, 1778), S(1803, 1765), S(1772, 1820), S(1734, 1807), S(1738, 1800), S(1710, 1784), 
S(-234, -127), S(197, -97), S(256, -84), S(142, -77), S(-213, 13), S(-120, 34), S(100, -24), S(53, -45), S(267, -80), S(96, 15), S(69, 19), S(225, -6), S(95, 14), S(89, 48), S(-77, 51), S(-270, 67), S(97, -12), S(150, 11), S(216, 2), S(49, 18), S(140, 9), S(261, 38), S(271, 35), S(-37, 17), S(6, -26), S(7, 36), S(89, 29), S(-26, 54), S(-20, 48), S(-13, 60), S(41, 42), S(-138, 23), S(-106, -30), S(98, -28), S(-39, 42), S(-129, 66), S(-130, 69), S(-90, 56), S(-66, 28), S(-159, 3), S(2, -39), S(31, -13), S(-37, 27), S(-82, 50), S(-89, 56), S(-60, 40), S(7, 11), S(-77, -4), S(9, -56), S(31, -22), S(-22, 17), S(-107, 35), S(-75, 37), S(-39, 21), S(36, -12), S(27, -45), S(-82, -95), S(63, -77), S(13, -37), S(-137, 2), S(-10, -40), S(-69, -10), S(51, -60), S(16, -103)};
constexpr int tempo = S(34, 33);
constexpr ValPair passer_coefs[] = {S(0, 0), S(-6, -17), S(-18, -4), S(-37, 29), S(4, 66), S(-2, 170), S(0, 0), S(0, 0)};
constexpr ValPair mobility[] = {S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), 
S(-68, -281), S(-19, -146), S(-8, -73), S(14, -27), S(48, -21), S(56, -5), S(75, 0), S(98, 1), S(119, -12), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), 
S(49, -154), S(-51, -168), S(-19, -96), S(15, -65), S(34, -49), S(54, -35), S(63, -22), S(73, -17), S(76, -7), S(87, -5), S(102, -10), S(141, -21), S(113, -6), S(169, -28), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), 
S(330, 104), S(-32, -102), S(5, -175), S(17, -134), S(28, -91), S(38, -75), S(49, -50), S(58, -47), S(75, -53), S(95, -52), S(116, -50), S(141, -47), S(162, -42), S(174, -45), S(206, -62), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), 
S(-100, -198), S(-75, -198), S(-47, -195), S(131, -254), S(288, -700), S(304, -447), S(296, -271), S(310, -257), S(309, -264), S(321, -250), S(325, -217), S(329, -197), S(336, -174), S(346, -166), S(349, -149), S(354, -126), S(361, -135), S(354, -103), S(361, -114), S(359, -94), S(368, -76), S(385, -79), S(374, -69), S(436, -100), S(465, -112), S(566, -178), S(315, -26), S(725, -257), 
S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)};
constexpr ValPair kingzone[] = {S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), 
S(3, -6), S(31, -1), S(34, -13), S(117, -37), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), 
S(0, -4), S(16, -7), S(41, -13), S(73, -23), S(57, -44), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), 
S(-6, 5), S(12, -9), S(43, -30), S(92, -25), S(168, -43), S(168, -46), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), 
S(27, -87), S(30, -52), S(26, -36), S(66, -31), S(100, -26), S(179, -49), S(461, -205), S(640, -307), S(374, 261), S(0, 0), S(0, 0), S(0, 0), 
S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)};

int gamephaseInc[12] = {0,1,1,2,4,0,0,1,1,2,4,0};
int mg_table[12][64];
int eg_table[12][64];

/* #define EVALHASH_SIZE (1<<18) //real size is 8*this in bytes (2MB here)
uint64_t evalhash[EVALHASH_SIZE]; */

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

    //clear eval hash table
    /* for (int i = 0; i < EVALHASH_SIZE; i++)
    {
        //"mate" value: if we get a wrong hit, avoid that position
        evalhash[i] = 192 - 32767; //getting lots of 0s in a hash is ultra-rare tho...
    } */
}

//"Expensive" mobility/king safety eval
void mob_ks_exp(Board &board, Value &mg, Value &eg)
{
    //calculate occupancy and pawn attacks for both sides
    U64 occ = board.occ();
    U64 wpawns = board.pieces(PieceType::PAWN, Color::WHITE);
    U64 bpawns = board.pieces(PieceType::PAWN, Color::BLACK);
    U64 wpawn_atk = wPawnAttacks(wpawns);
    U64 bpawn_atk = bPawnAttacks(bpawns);

    Square wking_sq = board.kingSq(Color::WHITE);
    Square bking_sq = board.kingSq(Color::BLACK);

    //calculate king zones
    U64 wking_zone = attacks::king(wking_sq);
    wking_zone |= wking_zone << 8;
    U64 bking_zone = attacks::king(bking_sq);
    bking_zone |= bking_zone >> 8;

    //calculate king virtual mobility scores (DISABLED)
    /* U64 white_vm = attacks::queen(wking_sq, occ);
    U64 black_vm = attacks::queen(board.kingSq(Color::BLACK), occ);
    //add it up! (NOTE: there is a bug here somewhere! but where???)
    mg += MG(mobility[5*28 + white_vm.count()]);
    mg -= MG(mobility[5*28 + black_vm.count()]);
    eg += EG(mobility[5*28 + white_vm.count()]);
    eg -= EG(mobility[5*28 + black_vm.count()]); */

    for (uint8_t sq = 0; sq < 64; ++sq) {
        uint8_t pc = (int)board.at<Piece>(sq);
        if (pc != (int)Piece::NONE) {
            //eval mobility
            //0: pawn; 1: knight; 2: bishop; 3: rook; 4: queen; 5: king
            const PieceType piece_type = board.at<PieceType>(sq);
            if (piece_type == PieceType::PAWN) continue; //skip pawns
            if (piece_type == PieceType::KING) continue; //skip king

            U64 atked_squares;
            switch((int)piece_type)
            {
                case 1: //knight
                    atked_squares = attacks::knight(sq); //don't think we have flip here!
                    break;
                case 2: //bishop
                    atked_squares = attacks::bishop(sq, occ);
                    break;
                case 3: //rook
                    atked_squares = attacks::rook(sq, occ);
                    break;
                case 4: //queen (king has already been handled)
                    atked_squares = attacks::queen(sq, occ);
                    break;
            }

            int8_t perspective = (pc >= 6) ? -1 : 1; //-1 if black, 1 if white

            //look at ENEMY king zone, not friendly! so its swapped on the right
            U64 king_zone_atk = atked_squares & ((perspective == 1) ? bking_zone : wking_zone);
            mg += MG(kingzone[piece_type * 12 + king_zone_atk.count()]) * perspective;
            eg += EG(kingzone[piece_type * 12 + king_zone_atk.count()]) * perspective;

            //exclude squares attacked by enemy pawns
            U64 enemy_pawn_atks = (perspective == -1) ? wpawn_atk : bpawn_atk;
            atked_squares &= ~enemy_pawn_atks;

            //add it up!
            ValPair cur_mob_score = mobility[piece_type * 28 + atked_squares.count()];
            mg += MG(cur_mob_score) * perspective;
            eg += EG(cur_mob_score) * perspective;
        }
    }
}

Value eval(Board board)
{
    //probe eval hash table
    /* uint64_t hash = board.hash();
    uint64_t evalhash_entry = evalhash[hash % EVALHASH_SIZE];
    if (!((evalhash_entry ^ hash) & 0xFFFFFFFF00000000)) //eval TT hit
        return (Value)(evalhash_entry & UINT32_MAX); */

    Value mg = 0, eg = 0;
    uint8_t mgPhase = 0;
    //TODO: scale endgame score based on pawns of stronger side (affine function)

    //create pawn bitboards (NOTE: square A1 is gonna be 0!!! no flipping required tho)
    U64 wpawns = board.pieces(PieceType::PAWN, Color::WHITE);
    U64 bpawns = board.pieces(PieceType::PAWN, Color::BLACK);
    //squares on the sides of pawns
    U64 wsides = ((wpawns << 1) & 0x7f7f7f7f7f7f7f7f) | ((wpawns >> 1) & 0xfefefefefefefefe);
    U64 bsides = ((bpawns << 1) & 0x7f7f7f7f7f7f7f7f) | ((bpawns >> 1) & 0xfefefefefefefefe);
    //White and Black frontspans | atk frontspans (hardcoded fill alg)
    U64 wspans = (wpawns | wsides) << 8;
    wspans |= wspans << 8;
    wspans |= wspans << 16;
    wspans |= wspans << 32;
    U64 bspans = (bpawns | bsides) >> 8;
    bspans |= bspans >> 8;
    bspans |= bspans >> 16;
    bspans |= bspans >> 32;
    //passer bitboards! finally!
    U64 wpassed = wpawns & ~bspans;
    U64 bpassed = bpawns & ~wspans;

    //add that stuff up! (careful about the flip!)
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

    //evaluate each piece (inefficient); also includes mobility!
    for (uint8_t sq = 0; sq < 64; ++sq) {
        uint8_t pc = (int)board.at<Piece>(sq);
        if (pc != (int)Piece::NONE) {
            int8_t perspective = (pc >= 6) ? -1 : 1; //-1 if black, 1 if white
            mg += mg_table[pc][sq] * perspective;
            eg += eg_table[pc][sq] * perspective;
            mgPhase += gamephaseInc[pc];
        }
    }

    //Do expensive mobility/king safety evaluation
    mob_ks_exp(board, mg, eg);

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

    //value that will be returned and stored in eval hash table
    Value final_value = (mg * mgPhase + eg * egPhase) / 24;
    //store it in eval hash table
    // evalhash[hash % EVALHASH_SIZE] = (uint64_t)(uint32_t)final_value | (hash & 0xFFFFFFFF00000000);
    return final_value;
}

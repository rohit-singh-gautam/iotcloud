-- /////////////////////////////////////////////////////////////////////////////////////////////
-- // Author: Rohit Jairaj Singh (rohit@singh.org.in)                                         //
-- // This program is free software: you can redistribute it and/or modify it under the terms //
-- // of the GNU General Public License as published by the Free Software Foundation, either  //
-- // version 3 of the License, or (at your option) any later version.                        //
-- //                                                                                         //
-- // This program is distributed in the hope that it will be useful, but WITHOUT ANY         //
-- // WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A         //
-- // PARTICULAR PURPOSE. See the GNU General Public License for more details.                //
-- //                                                                                         //
-- // You should have received a copy of the GNU General Public License along with this       //
-- // program. If not, see <https://www.gnu.org/licenses/>.                                   //
-- /////////////////////////////////////////////////////////////////////////////////////////////
-- CREATE USER iotuser;
-- CREATE DATABASE iot;
-- GRANT ALL PRIVILEGES ON DATABASE "iot" TO iotuser;

-- CREATE EXTENSION "uuid-ossp";
-- CREATE EXTENSION "pgcrypto";

DROP TABLE IF EXISTS PROTECTED_DEVICE CASCADE;
DROP TABLE IF EXISTS MODEL CASCADE;
DROP TABLE IF EXISTS COMPONENT CASCADE;
DROP TABLE IF EXISTS MODEL_COMPONENT CASCADE;
DROP TABLE IF EXISTS COUNTRY CASCADE;
DROP TABLE IF EXISTS USERS CASCADE;
DROP TABLE IF EXISTS PUBLIC_DEVICE CASCADE;
DROP TABLE IF EXISTS GROUPS CASCADE;
DROP TABLE IF EXISTS PERMISSIONS CASCADE;
DROP TABLE IF EXISTS AUTHORIZATIONS CASCADE;

DROP TYPE IF EXISTS PUBLIC_DEVICE_TYPE;

-- This is one enty per device
-- It contains all the information required for authentication
CREATE TABLE PROTECTED_DEVICE (
    ID              UUID    PRIMARY KEY,
    PUBLIC_KEY      BYTEA,
    SYMMETRIC_KEY   BYTEA,                  -- This will be created by authentication server
    KEY_EXPIRY      TIMESTAMP,
    MODELID         UUID
);


-- This information is only in cache
-- Updating database will be very costly
-- CREATE TABLE PROTECTED_DEVICE_COMPONENT {
--     DEVICE_ID       UUID,
--     COMPONENTINDEX  SMALLINT,
--     OPERATION       SMALLINT,
--     OP_VALUE        SMALLINT
-- };

-- Below table is heavily edited hence has to be optimized.
-- Avoiding primary key will help with it
CREATE TABLE PROTECTED_DEVICE_RANDOMIZATION (
    DEVICE_ID       UUID,
    RANDOM_ID       UUID   
);

CREATE INDEX PROTECTED_DEVICE_RANDOMIZATION_DEVICE_ID_INDEX ON PROTECTED_DEVICE_RANDOMIZATION USING HASH (DEVICE_ID);
CREATE INDEX PROTECTED_DEVICE_RANDOMIZATION_RANDOM_ID_INDEX ON PROTECTED_DEVICE_RANDOMIZATION USING HASH (RANDOM_ID);

CREATE TABLE MODEL (
    ID          INTEGER     PRIMARY KEY,
    NAME        VARCHAR,
    SPEC        JSON
);

CREATE TABLE COMPONENT (
    ID          INTEGER     PRIMARY KEY,
    NAME        VARCHAR,
    OPERATION   JSON
);

CREATE TABLE MODEL_COMPONENT (
    MODELID         INTEGER,
    COMPONENTINDEX  SMALLINT,
    COMPONENTID     INTEGER,
);

CREATE INDEX MODEL_COMPONENT_MODELID_INDEX ON MODEL_COMPONENT USING HASH (MODELID);
CREATE INDEX MODEL_COMPONENT_COMPONENTID_INDEX ON MODEL_COMPONENT USING HASH (COMPONENTID);

CREATE TABLE COUNTRY (
    NUMERICCODE NUMERIC(3),
    ALPHA3CODE  CHAR(3),
    ALPHA2CODE  CHAR(2),
    SHORTNAME   VARCHAR(48),
    PHONECODE   VARCHAR(4)
);

CREATE INDEX COUNTRY_NUMERICCODE_INDEX ON COUNTRY USING HASH (NUMERICCODE);
CREATE INDEX COUNTRY_ALPHA3CODE_INDEX ON COUNTRY USING HASH (ALPHA3CODE);
CREATE INDEX COUNTRY_ALPHA2CODE_INDEX ON COUNTRY USING HASH (ALPHA2CODE);
CREATE INDEX COUNTRY_SHORTNAME_INDEX ON COUNTRY USING HASH (SHORTNAME);
CREATE INDEX COUNTRY_PHONECODE_INDEX ON COUNTRY USING HASH (PHONECODE);

CREATE TABLE USERS (
    ID          UUID        PRIMARY KEY,
    EMAIL       VARCHAR     UNIQUE,
    MOBILE      VARCHAR(10) UNIQUE,
    PASSWORD    TEXT        NOT NULL,
    FIRSTNAME   VARCHAR(8),
    LASTNAME    VARCHAR(8),
    COUNTRY     NUMERIC(3)  NOT NULL,
    STATE       VARCHAR(8),
    PIN         NUMERIC(6),
    ADDRESS     TEXT
);

CREATE INDEX USER_EMAIL_INDEX ON USERS USING HASH (EMAIL);
CREATE INDEX USER_MOBILE_INDEX ON USERS USING HASH (MOBILE);

CREATE TYPE PUBLIC_DEVICE_TYPE AS ENUM (
  'ANDROID', 
  'WINDOWS', 
  'IPHONE');

CREATE TABLE PUBLIC_DEVICE (
    ID          UUID        PRIMARY KEY,
    TYPE        PUBLIC_DEVICE_TYPE,
    PUBLICKEY   BYTEA,
    NAME        VARCHAR,
    USERID      UUID,
);

CREATE TABLE GROUPS (
    ID          UUID        PRIMARY KEY,
    PARENT      UUID        NULL,
    NAME        VARCHAR(10),
    COUNTRY     NUMERIC(3),
    STATE       VARCHAR(8),
    PIN         NUMERIC(6),
    ADDRESS     TEXT
);

-- There will always a group with same UUID as USERSID
CREATE TABLE PERMISSIONS (
    USERID      UUID,
    GROUPID     UUID
);

CREATE TABLE AUTHORIZATIONS (
    DEVICEID    UUID,
    GROUPID     UUID
);

INSERT INTO COUNTRY (ALPHA2CODE, SHORTNAME, ALPHA3CODE, NUMERICCODE, PHONECODE) VALUES
('AF', 'Afghanistan', 'AFG', 4, 93),
('AL', 'Albania', 'ALB', 8, 355),
('DZ', 'Algeria', 'DZA', 12, 213),
('AS', 'American Samoa', 'ASM', 16, 1684),
('AD', 'Andorra', 'AND', 20, 376),
('AO', 'Angola', 'AGO', 24, 244),
('AI', 'Anguilla', 'AIA', 660, 1264),
('AQ', 'Antarctica', 'ATA', 10, 0),
('AG', 'Antigua and Barbuda', 'ATG', 28, 1268),
('AR', 'Argentina', 'ARG', 32, 54),
('AM', 'Armenia', 'ARM', 51, 374),
('AW', 'Aruba', 'ABW', 533, 297),
('AU', 'Australia', 'AUS', 36, 61),
('AT', 'Austria', 'AUT', 40, 43),
('AZ', 'Azerbaijan', 'AZE', 31, 994),
('BS', 'Bahamas', 'BHS', 44, 1242),
('BH', 'Bahrain', 'BHR', 48, 973),
('BD', 'Bangladesh', 'BGD', 50, 880),
('BB', 'Barbados', 'BRB', 52, 1246),
('BY', 'Belarus', 'BLR', 112, 375),
('BE', 'Belgium', 'BEL', 56, 32),
('BZ', 'Belize', 'BLZ', 84, 501),
('BJ', 'Benin', 'BEN', 204, 229),
('BM', 'Bermuda', 'BMU', 60, 1441),
('BT', 'Bhutan', 'BTN', 64, 975),
('BO', 'Bolivia', 'BOL', 68, 591),
('BA', 'Bosnia and Herzegovina', 'BIH', 70, 387),
('BW', 'Botswana', 'BWA', 72, 267),
('BV', 'Bouvet Island', 'BVT', 74, 0),
('BR', 'Brazil', 'BRA', 76, 55),
('IO', 'British Indian Ocean Territory', 'IOT', 86, 246),
('BN', 'Brunei Darussalam', 'BRN', 96, 673),
('BG', 'Bulgaria', 'BGR', 100, 359),
('BF', 'Burkina Faso', 'BFA', 854, 226),
('BI', 'Burundi', 'BDI', 108, 257),
('KH', 'Cambodia', 'KHM', 116, 855),
('CM', 'Cameroon', 'CMR', 120, 237),
('CA', 'Canada', 'CAN', 124, 1),
('CV', 'Cape Verde', 'CPV', 132, 238),
('KY', 'Cayman Islands', 'CYM', 136, 1345),
('CF', 'Central African Republic', 'CAF', 140, 236),
('TD', 'Chad', 'TCD', 148, 235),
('CL', 'Chile', 'CHL', 152, 56),
('CN', 'China', 'CHN', 156, 86),
('CX', 'Christmas Island', 'CXR', 162, 61),
('CC', 'Cocos (Keeling) Islands', NULL, NULL, 672),
('CO', 'Colombia', 'COL', 170, 57),
('KM', 'Comoros', 'COM', 174, 269),
('CG', 'Congo', 'COG', 178, 242),
('CD', 'Congo, the Democratic Republic of the', 'COD', 180, 242),
('CK', 'Cook Islands', 'COK', 184, 682),
('CR', 'Costa Rica', 'CRI', 188, 506),
('CI', 'Cote D''Ivoire', 'CIV', 384, 225),
('HR', 'Croatia', 'HRV', 191, 385),
('CU', 'Cuba', 'CUB', 192, 53),
('CY', 'Cyprus', 'CYP', 196, 357),
('CZ', 'Czech Republic', 'CZE', 203, 420),
('DK', 'Denmark', 'DNK', 208, 45),
('DJ', 'Djibouti', 'DJI', 262, 253),
('DM', 'Dominica', 'DMA', 212, 1767),
('DO', 'Dominican Republic', 'DOM', 214, 1),
('EC', 'Ecuador', 'ECU', 218, 593),
('EG', 'Egypt', 'EGY', 818, 20),
('SV', 'El Salvador', 'SLV', 222, 503),
('GQ', 'Equatorial Guinea', 'GNQ', 226, 240),
('ER', 'Eritrea', 'ERI', 232, 291),
('EE', 'Estonia', 'EST', 233, 372),
('ET', 'Ethiopia', 'ETH', 231, 251),
('FK', 'Falkland Islands (Malvinas)', 'FLK', 238, 500),
('FO', 'Faroe Islands', 'FRO', 234, 298),
('FJ', 'Fiji', 'FJI', 242, 679),
('FI', 'Finland', 'FIN', 246, 358),
('FR', 'France', 'FRA', 250, 33),
('GF', 'French Guiana', 'GUF', 254, 594),
('PF', 'French Polynesia', 'PYF', 258, 689),
('TF', 'French Southern Territories', 'ATF', 260, 0),
('GA', 'Gabon', 'GAB', 266, 241),
('GM', 'Gambia', 'GMB', 270, 220),
('GE', 'Georgia', 'GEO', 268, 995),
('DE', 'Germany', 'DEU', 276, 49),
('GH', 'Ghana', 'GHA', 288, 233),
('GI', 'Gibraltar', 'GIB', 292, 350),
('GR', 'Greece', 'GRC', 300, 30),
('GL', 'Greenland', 'GRL', 304, 299),
('GD', 'Grenada', 'GRD', 308, 1473),
('GP', 'Guadeloupe', 'GLP', 312, 590),
('GU', 'Guam', 'GUM', 316, 1671),
('GT', 'Guatemala', 'GTM', 320, 502),
('GN', 'Guinea', 'GIN', 324, 224),
('GW', 'Guinea-Bissau', 'GNB', 624, 245),
('GY', 'Guyana', 'GUY', 328, 592),
('HT', 'Haiti', 'HTI', 332, 509),
('HM', 'Heard Island and Mcdonald Islands', 'HMD', 334, 0),
('VA', 'Holy See (Vatican City State)', 'VAT', 336, 39),
('HN', 'Honduras', 'HND', 340, 504),
('HK', 'Hong Kong', 'HKG', 344, 852),
('HU', 'Hungary', 'HUN', 348, 36),
('IS', 'Iceland', 'ISL', 352, 354),
('IN', 'India', 'IND', 356, 91),
('ID', 'Indonesia', 'IDN', 360, 62),
('IR', 'Iran, Islamic Republic of', 'IRN', 364, 98),
('IQ', 'Iraq', 'IRQ', 368, 964),
('IE', 'Ireland', 'IRL', 372, 353),
('IL', 'Israel', 'ISR', 376, 972),
('IT', 'Italy', 'ITA', 380, 39),
('JM', 'Jamaica', 'JAM', 388, 1876),
('JP', 'Japan', 'JPN', 392, 81),
('JO', 'Jordan', 'JOR', 400, 962),
('KZ', 'Kazakhstan', 'KAZ', 398, 7),
('KE', 'Kenya', 'KEN', 404, 254),
('KI', 'Kiribati', 'KIR', 296, 686),
('KP', 'Korea, Democratic People''s Republic of', 'PRK', 408, 850),
('KR', 'Korea, Republic of', 'KOR', 410, 82),
('KW', 'Kuwait', 'KWT', 414, 965),
('KG', 'Kyrgyzstan', 'KGZ', 417, 996),
('LA', 'Lao People''s Democratic Republic', 'LAO', 418, 856),
('LV', 'Latvia', 'LVA', 428, 371),
('LB', 'Lebanon', 'LBN', 422, 961),
('LS', 'Lesotho', 'LSO', 426, 266),
('LR', 'Liberia', 'LBR', 430, 231),
('LY', 'Libyan Arab Jamahiriya', 'LBY', 434, 218),
('LI', 'Liechtenstein', 'LIE', 438, 423),
('LT', 'Lithuania', 'LTU', 440, 370),
('LU', 'Luxembourg', 'LUX', 442, 352),
('MO', 'Macao', 'MAC', 446, 853),
('MK', 'North Macedonia', 'MKD', 807, 389),
('MG', 'Madagascar', 'MDG', 450, 261),
('MW', 'Malawi', 'MWI', 454, 265),
('MY', 'Malaysia', 'MYS', 458, 60),
('MV', 'Maldives', 'MDV', 462, 960),
('ML', 'Mali', 'MLI', 466, 223),
('MT', 'Malta', 'MLT', 470, 356),
('MH', 'Marshall Islands', 'MHL', 584, 692),
('MQ', 'Martinique', 'MTQ', 474, 596),
('MR', 'Mauritania', 'MRT', 478, 222),
('MU', 'Mauritius', 'MUS', 480, 230),
('YT', 'Mayotte', 'MYT', 175, 269),
('MX', 'Mexico', 'MEX', 484, 52),
('FM', 'Micronesia, Federated States of', 'FSM', 583, 691),
('MD', 'Moldova, Republic of', 'MDA', 498, 373),
('MC', 'Monaco', 'MCO', 492, 377),
('MN', 'Mongolia', 'MNG', 496, 976),
('MS', 'Montserrat', 'MSR', 500, 1664),
('MA', 'Morocco', 'MAR', 504, 212),
('MZ', 'Mozambique', 'MOZ', 508, 258),
('MM', 'Myanmar', 'MMR', 104, 95),
('NA', 'Namibia', 'NAM', 516, 264),
('NR', 'Nauru', 'NRU', 520, 674),
('NP', 'Nepal', 'NPL', 524, 977),
('NL', 'Netherlands', 'NLD', 528, 31),
('AN', 'Netherlands Antilles', 'ANT', 530, 599),
('NC', 'New Caledonia', 'NCL', 540, 687),
('NZ', 'New Zealand', 'NZL', 554, 64),
('NI', 'Nicaragua', 'NIC', 558, 505),
('NE', 'Niger', 'NER', 562, 227),
('NG', 'Nigeria', 'NGA', 566, 234),
('NU', 'Niue', 'NIU', 570, 683),
('NF', 'Norfolk Island', 'NFK', 574, 672),
('MP', 'Northern Mariana Islands', 'MNP', 580, 1670),
('NO', 'Norway', 'NOR', 578, 47),
('OM', 'Oman', 'OMN', 512, 968),
('PK', 'Pakistan', 'PAK', 586, 92),
('PW', 'Palau', 'PLW', 585, 680),
('PS', 'Palestinian Territory, Occupied', NULL, NULL, 970),
('PA', 'Panama', 'PAN', 591, 507),
('PG', 'Papua New Guinea', 'PNG', 598, 675),
('PY', 'Paraguay', 'PRY', 600, 595),
('PE', 'Peru', 'PER', 604, 51),
('PH', 'Philippines', 'PHL', 608, 63),
('PN', 'Pitcairn', 'PCN', 612, 0),
('PL', 'Poland', 'POL', 616, 48),
('PT', 'Portugal', 'PRT', 620, 351),
('PR', 'Puerto Rico', 'PRI', 630, 1787),
('QA', 'Qatar', 'QAT', 634, 974),
('RE', 'Reunion', 'REU', 638, 262),
('RO', 'Romania', 'ROU', 642, 40),
('RU', 'Russian Federation', 'RUS', 643, 7),
('RW', 'Rwanda', 'RWA', 646, 250),
('SH', 'Saint Helena', 'SHN', 654, 290),
('KN', 'Saint Kitts and Nevis', 'KNA', 659, 1869),
('LC', 'Saint Lucia', 'LCA', 662, 1758),
('PM', 'Saint Pierre and Miquelon', 'SPM', 666, 508),
('VC', 'Saint Vincent and the Grenadines', 'VCT', 670, 1784),
('WS', 'Samoa', 'WSM', 882, 684),
('SM', 'San Marino', 'SMR', 674, 378),
('ST', 'Sao Tome and Principe', 'STP', 678, 239),
('SA', 'Saudi Arabia', 'SAU', 682, 966),
('SN', 'Senegal', 'SEN', 686, 221),
('RS', 'Serbia', 'SRB', 688, 381),
('SC', 'Seychelles', 'SYC', 690, 248),
('SL', 'Sierra Leone', 'SLE', 694, 232),
('SG', 'Singapore', 'SGP', 702, 65),
('SK', 'Slovakia', 'SVK', 703, 421),
('SI', 'Slovenia', 'SVN', 705, 386),
('SB', 'Solomon Islands', 'SLB', 90, 677),
('SO', 'Somalia', 'SOM', 706, 252),
('ZA', 'South Africa', 'ZAF', 710, 27),
('GS', 'South Georgia and the South Sandwich Islands', 'SGS', 239, 0),
('ES', 'Spain', 'ESP', 724, 34),
('LK', 'Sri Lanka', 'LKA', 144, 94),
('SD', 'Sudan', 'SDN', 736, 249),
('SR', 'Suriname', 'SUR', 740, 597),
('SJ', 'Svalbard and Jan Mayen', 'SJM', 744, 47),
('SZ', 'Swaziland', 'SWZ', 748, 268),
('SE', 'Sweden', 'SWE', 752, 46),
('CH', 'Switzerland', 'CHE', 756, 41),
('SY', 'Syrian Arab Republic', 'SYR', 760, 963),
('TW', 'Taiwan, Province of China', 'TWN', 158, 886),
('TJ', 'Tajikistan', 'TJK', 762, 992),
('TZ', 'Tanzania, United Republic of', 'TZA', 834, 255),
('TH', 'Thailand', 'THA', 764, 66),
('TL', 'Timor-Leste', 'TLS', 626, 670),
('TG', 'Togo', 'TGO', 768, 228),
('TK', 'Tokelau', 'TKL', 772, 690),
('TO', 'Tonga', 'TON', 776, 676),
('TT', 'Trinidad and Tobago', 'TTO', 780, 1868),
('TN', 'Tunisia', 'TUN', 788, 216),
('TR', 'Turkey', 'TUR', 792, 90),
('TM', 'Turkmenistan', 'TKM', 795, 993),
('TC', 'Turks and Caicos Islands', 'TCA', 796, 1649),
('TV', 'Tuvalu', 'TUV', 798, 688),
('UG', 'Uganda', 'UGA', 800, 256),
('UA', 'Ukraine', 'UKR', 804, 380),
('AE', 'United Arab Emirates', 'ARE', 784, 971),
('GB', 'United Kingdom', 'GBR', 826, 44),
('US', 'United States', 'USA', 840, 1),
('UM', 'United States Minor Outlying Islands', 'UMI', 581, 1),
('UY', 'Uruguay', 'URY', 858, 598),
('UZ', 'Uzbekistan', 'UZB', 860, 998),
('VU', 'Vanuatu', 'VUT', 548, 678),
('VE', 'Venezuela', 'VEN', 862, 58),
('VN', 'Viet Nam', 'VNM', 704, 84),
('VG', 'Virgin Islands, British', 'VGB', 92, 1284),
('VI', 'Virgin Islands, U.s.', 'VIR', 850, 1340),
('WF', 'Wallis and Futuna', 'WLF', 876, 681),
('EH', 'Western Sahara', 'ESH', 732, 212),
('YE', 'Yemen', 'YEM', 887, 967),
('ZM', 'Zambia', 'ZMB', 894, 260),
('ZW', 'Zimbabwe', 'ZWE', 716, 263),
('ME', 'Montenegro', 'MNE', 499, 382),
('XK', 'Kosovo', 'XKX', 0, 383),
('AX', 'Aland Islands', 'ALA', '248', '358'),
('BQ', 'Bonaire, Sint Eustatius and Saba', 'BES', '535', '599'),
('CW', 'Curacao', 'CUW', '531', '599'),
('GG', 'Guernsey', 'GGY', '831', '44'),
('IM', 'Isle of Man', 'IMN', '833', '44'),
('JE', 'Jersey', 'JEY', '832', '44'),
('BL', 'Saint Barthelemy', 'BLM', '652', '590'),
('MF', 'Saint Martin', 'MAF', '663', '590'),
('SX', 'Sint Maarten', 'SXM', '534', '1'),
('SS', 'South Sudan', 'SSD', '728', '211');

INSERT INTO COMPONENT (ID, NAME, OPERATION) VALUES
(1, 'switch', '{values: ["on", "off"]}'),
(2, 'auto switch', '{values: ["on", "auto", "off"]}'),
(3, 'variable 256', '{ranges: {low: "0", high: "255"}}'),
(4, 'variable 1024', '{ranges: {low: "0", high: "1023"}}'),
(5, 'RGB 256', '{Red: {low: "0", high: "255"}, Blue: {low: "0", high: "255"}, Green: {low: "0", high: "255"}}'),
(6, 'RGB 1024', '{Red: {low: "0", high: "1023"}, Blue: {low: "0", high: "1023"}, Green: {low: "0", high: "1023"}}');

INSERT INTO MODEL (ID, NAME, SPEC) VALUES
(1, 'switch', '{}'),
(2, '2 way switch', '{}'),
(3, '3 way switch', '{}'),
(4, '4 way switch', '{}'),
(5, '5 way switch', '{}'),
(6, '6 way switch', '{}'),
(7, '7 way switch', '{}'),
(8, '8 way switch', '{}'),
(9, '9 way switch', '{}'),
(10, '10 way switch', '{}'),
(11, '11 way switch', '{}'),
(12, '12 way switch', '{}'),
(13, '13 way switch', '{}'),
(14, '14 way switch', '{}'),
(15, '15 way switch', '{}'),
(16, '16 way switch', '{}'),
(17, 'RGB Light', '{}'),
(18, 'Fine RGB Light', '{}'),;


INSERT INTO MODEL (MODELID, COMPONENTINDEX, COMPONENTID) VALUES
(1, 0, 1),
(2, 0, 1), (2, 1, 1),
(3, 0, 1), (3, 1, 1), (3, 2, 1),
(4, 0, 1), (4, 1, 1), (4, 2, 1), (4, 3, 1),
(5, 0, 1), (5, 1, 1), (5, 2, 1), (5, 3, 1), (5, 4, 1),
(6, 0, 1), (6, 1, 1), (6, 2, 1), (6, 3, 1), (6, 4, 1), (6, 5, 1),
(7, 0, 1), (7, 1, 1), (7, 2, 1), (7, 3, 1), (7, 4, 1), (7, 5, 1), (7, 6, 1),
(8, 0, 1), (8, 1, 1), (8, 2, 1), (8, 3, 1), (8, 4, 1), (8, 5, 1), (8, 6, 1), (8, 7, 1),
(9, 0, 1), (9, 1, 1), (9, 2, 1), (9, 3, 1), (9, 4, 1), (9, 5, 1), (9, 6, 1), (9, 7, 1), (9, 8, 1),
(10, 0, 1), (10, 1, 1), (10, 2, 1), (10, 3, 1), (10, 4, 1), (10, 5, 1), (10, 6, 1), (10, 7, 1), (10, 8, 1), (10, 9, 1),
(11, 0, 1), (11, 1, 1), (11, 2, 1), (11, 3, 1), (11, 4, 1), (11, 5, 1),
    (11, 6, 1), (11, 7, 1), (11, 8, 1), (11, 9, 1), (11, 10, 1),
(12, 0, 1), (12, 1, 1), (12, 2, 1), (12, 3, 1), (12, 4, 1), (12, 5, 1),
    (12, 6, 1), (12, 7, 1), (12, 8, 1), (12, 9, 1), (12, 10, 1), (12, 11, 1),
(13, 0, 1), (13, 1, 1), (13, 2, 1), (13, 3, 1), (13, 4, 1), (13, 5, 1), (13, 6, 1),
    (13, 7, 1), (13, 8, 1), (13, 9, 1), (13, 10, 1), (13, 11, 1), (13, 12, 1),
(14, 0, 1), (14, 1, 1), (14, 2, 1), (14, 3, 1), (14, 4, 1), (14, 5, 1), (14, 6, 1),
    (14, 7, 1), (14, 8, 1), (14, 9, 1), (14, 10, 1), (14, 11, 1), (14, 12, 1), (14, 13, 1),
(15, 0, 1), (15, 1, 1), (15, 2, 1), (15, 3, 1), (15, 4, 1), (15, 5, 1), (15, 6, 1), (15, 7, 1),
    (15, 8, 1), (15, 9, 1), (15, 10, 1), (15, 11, 1), (15, 12, 1), (15, 13, 1), (15, 14, 1),
(16, 0, 1), (16, 1, 1), (16, 2, 1), (16, 3, 1), (16, 4, 1), (16, 5, 1), (16, 6, 1), (16, 7, 1),
    (16, 8, 1), (16, 9, 1), (16, 10, 1), (16, 11, 1), (16, 12, 1), (16, 13, 1), (16, 14, 1), (16, 15, 1),
(17, 0, 5),
(18, 0, 6);
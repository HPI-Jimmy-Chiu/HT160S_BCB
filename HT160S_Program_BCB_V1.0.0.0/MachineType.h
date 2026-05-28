//---------------------------------------------------------------------------
#ifndef MachineTypeH
#define MachineTypeH
//---------------------------------------------------------------------------
// Build option imported from HT172 20260420.
// Comment this define for real machine hardware I/O.
#define SOFT_SIMULATE

#define USE_CompareCommandPos

const int AUTOMATION_MAX_DATA=260;

// Customer code definitions imported from HT172 20260420.
#define CC_HONPREC_QC             0

#define CC_Morningcore          766
#define CC_Paceis               767
#define CC_Sigmastar            768
#define CC_BRAVETEK             769
#define CC_DJI_SZ               770

#define CC_NXP_TJ               780
#define CC_XINYUN               781

#define CC_FOREHOPE_NINGBO      790
#define CC_SJ_Semiconductor     791
#define CC_SJ_Semiconductor_OS  792
#define CC_30JAVEE              793
#define CC_CAMBRICON            794
#define CC_VATE                 795
#define CC_ASIAOPTICAL          796
#define CC_HXYSEMI              797
#define CC_LEADYO               798
#define CC_DENGLIN              799

#define CC_BROADCOM_US          800
#define CC_AVAGO_Korea          810
#define CC_TSMC_TAINAN          820
#define CC_TSMC_HSINCHU         825
#define CC_HTKJXA_CHINA         830
#define CC_RF360                831

#define CC_RIGGER_MICRO         834
#define CC_XINITECH             835
#define CC_GIGA_FORCE_Zhejiang  836
#define CC_GIGA_FORCE_Shanghai  837
#define CC_CSAMQ                838
#define CC_GONGJIN_SHANGHAI     839

#define CC_SANDISK_CHINA        840
#define CC_GONGJIN_SUZHOU       841
#define CC_Mathilda             842
#define CC_DoosanTesna          843
#define CC_Renesas_M            844
#define CC_STK                  845
#define CC_Renesas              846
#define CC_THINE                847
#define CC_SINOICTECH           848
#define CC_ChipOn               849

#define CC_ChipMos_TAINAN       850
#define CC_ChipMos_ZHUBEI       851
#define CC_ITS                  852
#define CC_NEXPERIA_Guangdong   853
#define CC_Atec_Semiconductor   854
#define CC_JSSI_Semiconductor   855
#define CC_Microchip_FR         856
#define CC_Indie_US             857
#define CC_CETC                 858
#define CC_XDXCT                859

#define CC_MAXIM_THAILAND       860
#define CC_Microchip_Thailand   861
#define CC_Microchip_Philippines 862
#define CC_Microchip_China      863
#define CC_Microchip_US         864
#define CC_HANA_MICRON          865
#define CC_ITestInc             866
#define CC_EMemory              867
#define CC_CYUEAN               868
#define CC_PANTHER              869

#define CC_ARDENTEC             870
#define CC_FULCAP               871
#define CC_AOSL                 872
#define CC_Nuvoton_Israel       873
#define CC_GT                   874
#define CC_Novatek              875
#define CC_Sunplus              876
#define CC_Amlogic              877
#define CC_Higon                878
#define CC_Kingston             879

#define CC_Spreadtrum           880
#define CC_Amazon               881
#define CC_Murata               882
#define CC_Goertek              883
#define CC_FMSH                 884
#define CC_HDSC                 885
#define CC_SANECHIPS            886
#define CC_HABANA               887
#define CC_GIS                  888
#define CC_CENTER               889

#define CC_UMC                  890
#define CC_ATEC                 891
#define CC_WINSTEK              892
#define CC_SANAN                893
#define CC_UTAC_TW              894
#define CC_BARUN                895
#define CC_SIGURD_SUZHOU        896
#define CC_SILTERRA_CHINKIANG   897
#define CC_AMD_SUZHOU           898
#define CC_YTEC                 899

#define CC_ISE_US               901
#define CC_HYGEIA_SUZHOU        902
#define CC_JSI_HAOXING          903
#define CC_JINGJIAWEI_CHANGSHA  904
#define CC_ISE_SH               905

#define CC_INTEL_IL             906
#define CC_IBM_CANADA           907
#define CC_PGC                  908
#define CC_ITESTSEMI            909

#define CC_SPIL_SHINCHU         910
#define CC_SPIL_TAICHUNG_LOGIC  911
#define CC_SPIL_CHINA_SUZHOU    912
#define CC_HUAWEI               913
#define CC_ANST                 914
#define CC_VTEST                915
#define CC_TFME_CHINA           916
#define CC_JCET_5               917
#define CC_BOJIAN               918
#define CC_VTEST_Shanghai       919

#define CC_KYEC_CHEN            920
#define CC_KYEC_LEE             921
#define CC_KYEC_JCTHIU          922
#define CC_DL_TEK               923
#define CC_KYEC_XILINX          924
#define CC_KYEC_STM             925
#define CC_Advantest_GE         926

#define CC_ASE_KaohSiung_K12    929
#define CC_ASE_SG               930
#define CC_ASE_JP               931
#define CC_ASE_Korea            932
#define CC_ASE_CL               933
#define CC_ASE_SH               934
#define CC_ASE_N                935
#define CC_ASE_KaohSiung        936
#define CC_ASE_M                937
#define CC_ASE_KaohSiung_K3     938
#define CC_ASE_KaohSiung_K11    939

#define CC_UTAC                 940
#define CC_SIGURD_HUKOU         941
#define CC_RFMD_BEIJING         942
#define CC_SCC                  943
#define CC_SCS                  944
#define CC_SIGURD_ChungXing     945
#define CC_SIGURD_PeiXing       946
#define CC_SCK                  947
#define CC_RFMD_USA             948
#define CC_ASE_KS               949

#define CC_APTOS                950
#define CC_WINBOND              951
#define CC_G_Link               952
#define CC_AMKOR                953
#define CC_GIGA                 954
#define CC_LINGSEN              955
#define CC_Greatek              956
#define CC_PTI                  957
#define CC_THAILIN              958
#define CC_JCET                 959

#define CC_OSE                  960
#define CC_MTI                  961
#define CC_NUVOTON              962
#define CC_UPRTEK               963
#define CC_Eutrend              964
#define CC_TICP                 965
#define CC_THEIL                966
#define CC_TERAPOWER            967
#define CC_RICHTEK              968
#define CC_TSI                  969

#define CC_GIGAS                970
#define CC_AMKOR_Korea          971
#define CC_AMKOR_China          972
#define CC_AMKOR_Japan          973
#define CC_AMKOR_Philippines    974

#define CC_AnalogDevice_Phil    976

#define CC_INTEL_M              979
#define CC_CARSEM_M             980
#define CC_UNISEM_M             981
#define CC_AMD_M                982
#define CC_INARI_M              983
#define CC_ONSEMI_M             984

#define CC_MAXIM                985
#define CC_MARVELL              986
#define CC_ATMEL                987
#define CC_USI                  988
#define CC_DYNACARD             989

#define CC_CYPRESS              990
#define CC_GERADTECH_CHINA      991
#define CC_GM_TEST              992
#define CC_I_TECH               993
#define CC_WIN_PAC              994
#define CC_SILICON_LABS_SG      995
#define CC_SILICON_LABS_SZ      996
#define CC_Altera_USA           997
#define CC_ETRENDTECH           998
#define CC_QUALCOMM             999

#define MAX_IONFAN 8

#ifndef HT160S_DEFAULT_CUSTOMER_CODE
#define HT160S_DEFAULT_CUSTOMER_CODE CC_HONPREC_QC
#endif

//---------------------------------------------------------------------------
#endif
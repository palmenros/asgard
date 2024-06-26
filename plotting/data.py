# PageRank
uniformity_way_vs_inter_intra = {
    'cache_slice_sizes': [2097152, 4194304, 8388608, 16777216, 33554432, 67108864, 134217728],
    'way_partition_misses': [417267, 143304, 70425, 66798, 66798, 66798, 66798],
    'inter_intra_node_misses': [521348, 66975, 66798, 66798, 66798, 66798, 66798],
    'way_partition_accesses': [[505499, 491457, 432123, 489027, 455694, 465765, 452648, 462803],
                               [505499, 491457, 432123, 489027, 455694, 465765, 452648, 462803],
                               [505499, 491457, 432123, 489027, 455694, 465765, 452648, 462803],
                               [505499, 491457, 432123, 489027, 455694, 465765, 452648, 462803],
                               [505499, 491457, 432123, 489027, 455694, 465765, 452648, 462803],
                               [505499, 491457, 432123, 489027, 455694, 465765, 452648, 462803],
                               [505499, 491457, 432123, 489027, 455694, 465765, 452648, 462803]],
    'inter_intra_node_accesses': [[2655115, 1099901, 0, 0, 0, 0, 0, 0], [2337628, 1417388, 0, 0, 0, 0, 0, 0],
                                  [2400646, 1354370, 0, 0, 0, 0, 0, 0], [2640729, 1114287, 0, 0, 0, 0, 0, 0],
                                  [2209661, 1545355, 0, 0, 0, 0, 0, 0], [1950805, 1804211, 0, 0, 0, 0, 0, 0],
                                  [2134382, 1620634, 0, 0, 0, 0, 0, 0]],
    'total_analyzed': 18806069,
}

# PageRank
intra_vs_way_partitioning = {
    'cache_slice_sizes': [8388608, 16777216, 33554432, 67108864, 134217728, 268435456, 536870912],
    'way_partition_misses': [799845, 556926, 332223, 182261, 155620, 100421, 77159],
    'intra_node_misses': [561107, 555955, 252836, 66807, 66798, 66798, 66798],
    'way_partition_accesses': [3755016, 3755016, 3755016, 3755016, 3755016, 3755016, 3755016],
    'intra_node_accesses': [3755016, 3755016, 3755016, 3755016, 3755016, 3755016, 3755016],
    'total_analyzed': 18806069,
}

# PageRank
inter_vs_cluster_way_partitioning_vs_inter_intra = {
    1: {
        'cache_slice_sizes': [2097152, 4194304, 8388608, 16777216, 33554432, 67108864, 134217728],
        'inter_node_misses': [555955, 252836, 66807, 66798, 66798, 66798, 66798],
        'way_partition_misses': [556926, 332223, 182261, 155620, 100421, 77159, 67841],
        'inter_intra_misses': [555955, 252836, 66807, 66798, 66798, 66798, 66798],
        'inter_node_accesses': [[3755016], [3755016], [3755016], [3755016], [3755016], [3755016], [3755016]],
        'way_partition_accesses': [[505499, 491457, 432123, 489027, 455694, 465765, 452648, 462803],
                                   [505499, 491457, 432123, 489027, 455694, 465765, 452648, 462803],
                                   [505499, 491457, 432123, 489027, 455694, 465765, 452648, 462803],
                                   [505499, 491457, 432123, 489027, 455694, 465765, 452648, 462803],
                                   [505499, 491457, 432123, 489027, 455694, 465765, 452648, 462803],
                                   [505499, 491457, 432123, 489027, 455694, 465765, 452648, 462803],
                                   [505499, 491457, 432123, 489027, 455694, 465765, 452648, 462803]],
        'inter_intra_accesses': [[3755016, 0, 0, 0, 0, 0, 0, 0], [3755016, 0, 0, 0, 0, 0, 0, 0],
                                 [3755016, 0, 0, 0, 0, 0, 0, 0], [3755016, 0, 0, 0, 0, 0, 0, 0],
                                 [3755016, 0, 0, 0, 0, 0, 0, 0], [3755016, 0, 0, 0, 0, 0, 0, 0],
                                 [3755016, 0, 0, 0, 0, 0, 0, 0]],
        'total_analyzed': 18806069
    },
    2: {
        'cache_slice_sizes': [2097152, 4194304, 8388608, 16777216, 33554432, 67108864, 134217728],
        'inter_node_misses': [252836, 66807, 66798, 66798, 66798, 66798, 66798],
        'way_partition_misses': [273796, 107604, 78472, 75447, 66798, 66798, 66798],
        'inter_intra_misses': [252836, 66807, 66798, 66798, 66798, 66798, 66798],
        'inter_node_accesses': [[2336007, 1419009], [2067329, 1687687], [2226252, 1528764], [1808729, 1946287],
                                [1770436, 1984580], [848095, 2906921], [1929695, 1825321]],
        'way_partition_accesses': [[505499, 491457, 432123, 489027, 455694, 465765, 452648, 462803],
                                   [505499, 491457, 432123, 489027, 455694, 465765, 452648, 462803],
                                   [505499, 491457, 432123, 489027, 455694, 465765, 452648, 462803],
                                   [505499, 491457, 432123, 489027, 455694, 465765, 452648, 462803],
                                   [505499, 491457, 432123, 489027, 455694, 465765, 452648, 462803],
                                   [505499, 491457, 432123, 489027, 455694, 465765, 452648, 462803],
                                   [505499, 491457, 432123, 489027, 455694, 465765, 452648, 462803]],
        'inter_intra_accesses': [[2336007, 1419009, 0, 0, 0, 0, 0, 0], [2067329, 1687687, 0, 0, 0, 0, 0, 0],
                                 [2226252, 1528764, 0, 0, 0, 0, 0, 0], [1808729, 1946287, 0, 0, 0, 0, 0, 0],
                                 [1770436, 1984580, 0, 0, 0, 0, 0, 0], [848095, 2906921, 0, 0, 0, 0, 0, 0],
                                 [1929695, 1825321, 0, 0, 0, 0, 0, 0]],
        'total_analyzed': 18806069
    },
    4: {
        'cache_slice_sizes': [2097152, 4194304, 8388608, 16777216, 33554432, 67108864, 134217728],
        'inter_node_misses': [66807, 66798, 66798, 66798, 66798, 66798, 66798],
        'way_partition_misses': [75971, 66802, 66798, 66798, 66798, 66798, 66798],
        'inter_intra_misses': [66807, 66798, 66798, 66798, 66798, 66798, 66798],
        'inter_node_accesses': [[1180838, 886491, 1155169, 532518], [1214291, 1011961, 853038, 675726],
                                [1036755, 771974, 1189497, 756790], [703712, 1066724, 1105017, 879563],
                                [573723, 274372, 1196713, 1710208], [559386, 1370309, 288709, 1536612],
                                [452752, 376396, 1476943, 1448925]],
        'way_partition_accesses': [[505499, 491457, 432123, 489027, 455694, 465765, 452648, 462803],
                                   [505499, 491457, 432123, 489027, 455694, 465765, 452648, 462803],
                                   [505499, 491457, 432123, 489027, 455694, 465765, 452648, 462803],
                                   [505499, 491457, 432123, 489027, 455694, 465765, 452648, 462803],
                                   [505499, 491457, 432123, 489027, 455694, 465765, 452648, 462803],
                                   [505499, 491457, 432123, 489027, 455694, 465765, 452648, 462803],
                                   [505499, 491457, 432123, 489027, 455694, 465765, 452648, 462803]],
        'inter_intra_accesses': [[1180838, 886491, 1155169, 532518, 0, 0, 0, 0],
                                 [1214291, 1011961, 853038, 675726, 0, 0, 0, 0],
                                 [1036755, 771974, 1189497, 756790, 0, 0, 0, 0],
                                 [703712, 1066724, 1105017, 879563, 0, 0, 0, 0],
                                 [573723, 274372, 1196713, 1710208, 0, 0, 0, 0],
                                 [559386, 1370309, 288709, 1536612, 0, 0, 0, 0],
                                 [452752, 376396, 1476943, 1448925, 0, 0, 0, 0]],
        'total_analyzed': 18806069
    },
    8: {
        'cache_slice_sizes': [2097152, 4194304, 8388608, 16777216, 33554432, 67108864, 134217728],
        'inter_node_misses': [66798, 66798, 66798, 66798, 66798, 66798, 66798],
        'way_partition_misses': [66798, 66798, 66798, 66798, 66798, 66798, 66798],
        'inter_intra_misses': [66798, 66798, 66798, 66798, 66798, 66798, 66798],
        'inter_node_accesses': [[854928, 359363, 716440, 295521, 325910, 527128, 438729, 236997],
                                [553066, 483689, 439731, 332243, 661225, 528272, 413307, 343483],
                                [365588, 338124, 474444, 592280, 671167, 433850, 715053, 164510],
                                [442619, 131104, 125728, 148644, 261093, 935620, 979289, 730919],
                                [405746, 153640, 772327, 597982, 167977, 120732, 424386, 1112226],
                                [170720, 282032, 7855, 368541, 388666, 1088277, 280854, 1168071],
                                [451600, 8852, 550, 291100, 1152, 367544, 1476393, 1157825]],
        'way_partition_accesses': [[505499, 491457, 432123, 489027, 455694, 465765, 452648, 462803],
                                   [505499, 491457, 432123, 489027, 455694, 465765, 452648, 462803],
                                   [505499, 491457, 432123, 489027, 455694, 465765, 452648, 462803],
                                   [505499, 491457, 432123, 489027, 455694, 465765, 452648, 462803],
                                   [505499, 491457, 432123, 489027, 455694, 465765, 452648, 462803],
                                   [505499, 491457, 432123, 489027, 455694, 465765, 452648, 462803],
                                   [505499, 491457, 432123, 489027, 455694, 465765, 452648, 462803]],
        'inter_intra_accesses': [[854928, 359363, 716440, 295521, 325910, 527128, 438729, 236997],
                                 [553066, 483689, 439731, 332243, 661225, 528272, 413307, 343483],
                                 [365588, 338124, 474444, 592280, 671167, 433850, 715053, 164510],
                                 [442619, 131104, 125728, 148644, 261093, 935620, 979289, 730919],
                                 [405746, 153640, 772327, 597982, 167977, 120732, 424386, 1112226],
                                 [170720, 282032, 7855, 368541, 388666, 1088277, 280854, 1168071],
                                 [451600, 8852, 550, 291100, 1152, 367544, 1476393, 1157825]],
        'total_analyzed': 18806069
    },
}

inter_vs_cluster_way_partitioning_vs_inter_intra_bfs = {
    1: {
        'cache_slice_sizes': [2097152, 4194304, 8388608, 16777216, 33554432, 67108864, 134217728],
        'inter_node_misses': [343223, 332224, 260938, 198118, 197809, 197809, 197809],
        'way_partition_misses': [340140, 311702, 258988, 247851, 199398, 198329, 198022],
        'inter_intra_misses': [343223, 332224, 260938, 198118, 197809, 197809, 197809],
        'inter_node_accesses': [[349226], [349226], [349226], [349226], [349226], [349226], [349226]],
        'way_partition_accesses': [[43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021]],
        'inter_intra_accesses': [[349226, 0, 0, 0, 0, 0, 0, 0], [349226, 0, 0, 0, 0, 0, 0, 0],
                                 [349226, 0, 0, 0, 0, 0, 0, 0], [349226, 0, 0, 0, 0, 0, 0, 0],
                                 [349226, 0, 0, 0, 0, 0, 0, 0], [349226, 0, 0, 0, 0, 0, 0, 0],
                                 [349226, 0, 0, 0, 0, 0, 0, 0]],
        'total_analyzed': 6895840
    },
    2: {
        'cache_slice_sizes': [2097152, 4194304, 8388608, 16777216, 33554432, 67108864, 134217728],
        'inter_node_misses': [332224, 260938, 198118, 197809, 197809, 197809, 197809],
        'way_partition_misses': [301366, 262941, 209468, 200304, 197817, 197809, 197809],
        'inter_intra_misses': [332224, 260938, 198118, 197809, 197809, 197809, 197809],
        'inter_node_accesses': [[174233, 174993], [185491, 163735], [185874, 163352], [239268, 109958],
                                [170485, 178741], [200564, 148662], [114147, 235079]],
        'way_partition_accesses': [[43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021]],
        'inter_intra_accesses': [[174233, 174993, 0, 0, 0, 0, 0, 0], [185491, 163735, 0, 0, 0, 0, 0, 0],
                                 [185874, 163352, 0, 0, 0, 0, 0, 0], [239268, 109958, 0, 0, 0, 0, 0, 0],
                                 [170485, 178741, 0, 0, 0, 0, 0, 0], [200564, 148662, 0, 0, 0, 0, 0, 0],
                                 [114147, 235079, 0, 0, 0, 0, 0, 0]],
        'total_analyzed': 6895840
    },
    4: {
        'cache_slice_sizes': [2097152, 4194304, 8388608, 16777216, 33554432, 67108864, 134217728],
        'inter_node_misses': [260938, 198118, 197809, 197809, 197809, 197809, 197809],
        'way_partition_misses': [241698, 204367, 197857, 197809, 197809, 197809, 197809],
        'inter_intra_misses': [260938, 198118, 197809, 197809, 197809, 197809, 197809],
        'inter_node_accesses': [[90857, 94634, 83376, 80359], [93948, 91926, 91543, 71809],
                                [124123, 115145, 61751, 48207], [131564, 38921, 107704, 71037],
                                [41981, 158583, 128504, 20158], [39745, 74402, 160819, 74260],
                                [12156, 161993, 101991, 73086]],
        'way_partition_accesses': [[43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021]],
        'inter_intra_accesses': [[90857, 94634, 83376, 80359, 0, 0, 0, 0], [93948, 91926, 91543, 71809, 0, 0, 0, 0],
                                 [124123, 115145, 61751, 48207, 0, 0, 0, 0], [131564, 38921, 107704, 71037, 0, 0, 0, 0],
                                 [41981, 158583, 128504, 20158, 0, 0, 0, 0], [39745, 74402, 160819, 74260, 0, 0, 0, 0],
                                 [12156, 161993, 101991, 73086, 0, 0, 0, 0]],
        'total_analyzed': 6895840
    },
    8: {
        'cache_slice_sizes': [2097152, 4194304, 8388608, 16777216, 33554432, 67108864, 134217728],
        'inter_node_misses': [198118, 197809, 197809, 197809, 197809, 197809, 197809],
        'way_partition_misses': [198118, 197809, 197809, 197809, 197809, 197809, 197809],
        'inter_intra_misses': [198118, 197809, 197809, 197809, 197809, 197809, 197809],
        'inter_node_accesses': [[46607, 47341, 46927, 44999, 44250, 47293, 36449, 35360],
                                [61000, 63123, 63866, 51279, 32948, 28803, 27677, 20530],
                                [66462, 65102, 22873, 16048, 57661, 50043, 38878, 32159],
                                [10729, 31252, 94474, 64109, 120835, 7669, 13230, 6928],
                                [38016, 1729, 65934, 8468, 3965, 156854, 62570, 11690],
                                [6751, 5405, 156752, 5241, 32994, 68997, 4067, 69019],
                                [8025, 2341, 826, 61554, 4131, 159652, 101165, 11532]],
        'way_partition_accesses': [[43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021]],
        'inter_intra_accesses': [[46607, 47341, 46927, 44999, 44250, 47293, 36449, 35360],
                                 [61000, 63123, 63866, 51279, 32948, 28803, 27677, 20530],
                                 [66462, 65102, 22873, 16048, 57661, 50043, 38878, 32159],
                                 [10729, 31252, 94474, 64109, 120835, 7669, 13230, 6928],
                                 [38016, 1729, 65934, 8468, 3965, 156854, 62570, 11690],
                                 [6751, 5405, 156752, 5241, 32994, 68997, 4067, 69019],
                                 [8025, 2341, 826, 61554, 4131, 159652, 101165, 11532]],
        'total_analyzed': 6895840
    },
}

OLD_v2 = {
    1: {
        'cache_slice_sizes': [2097152, 4194304, 8388608, 16777216, 33554432, 67108864, 134217728],
        'inter_node_misses': [343223, 332224, 260938, 198118, 197809, 197809, 197809],
        'way_partition_misses': [345798, 344893, 340140, 311702, 258988, 247851, 199398],
        'inter_intra_misses': [343223, 332224, 260938, 198118, 197809, 197809, 197809],
        'inter_node_accesses': [[349226], [349226], [349226], [349226], [349226], [349226], [349226]],
        'way_partition_accesses': [[43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021]],
        'inter_intra_accesses': [[349226, 0, 0, 0, 0, 0, 0, 0], [349226, 0, 0, 0, 0, 0, 0, 0],
                                 [349226, 0, 0, 0, 0, 0, 0, 0], [349226, 0, 0, 0, 0, 0, 0, 0],
                                 [349226, 0, 0, 0, 0, 0, 0, 0], [349226, 0, 0, 0, 0, 0, 0, 0],
                                 [349226, 0, 0, 0, 0, 0, 0, 0]],
        'total_analyzed': 6895840
    },
    2: {
        'cache_slice_sizes': [2097152, 4194304, 8388608, 16777216, 33554432, 67108864, 134217728],
        'inter_node_misses': [332224, 260938, 198118, 197809, 197809, 197809, 197809],
        'way_partition_misses': [344661, 343468, 301366, 262941, 209468, 200304, 197817],
        'inter_intra_misses': [332224, 260938, 198118, 197809, 197809, 197809, 197809],
        'inter_node_accesses': [[174233, 174993], [185491, 163735], [185874, 163352], [239268, 109958],
                                [170485, 178741], [200564, 148662], [114147, 235079]],
        'way_partition_accesses': [[43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021]],
        'inter_intra_accesses': [[174233, 174993, 0, 0, 0, 0, 0, 0], [185491, 163735, 0, 0, 0, 0, 0, 0],
                                 [185874, 163352, 0, 0, 0, 0, 0, 0], [239268, 109958, 0, 0, 0, 0, 0, 0],
                                 [170485, 178741, 0, 0, 0, 0, 0, 0], [200564, 148662, 0, 0, 0, 0, 0, 0],
                                 [114147, 235079, 0, 0, 0, 0, 0, 0]],
        'total_analyzed': 6895840
    },
    4: {
        'cache_slice_sizes': [2097152, 4194304, 8388608, 16777216, 33554432, 67108864, 134217728],
        'inter_node_misses': [260938, 198118, 197809, 197809, 197809, 197809, 197809],
        'way_partition_misses': [343327, 322640, 241698, 204367, 197857, 197809, 197809],
        'inter_intra_misses': [260938, 198118, 197809, 197809, 197809, 197809, 197809],
        'inter_node_accesses': [[90857, 94634, 83376, 80359], [93948, 91926, 91543, 71809],
                                [124123, 115145, 61751, 48207], [131564, 38921, 107704, 71037],
                                [41981, 158583, 128504, 20158], [39745, 74402, 160819, 74260],
                                [12156, 161993, 101991, 73086]],
        'way_partition_accesses': [[43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021]],
        'inter_intra_accesses': [[90857, 94634, 83376, 80359, 0, 0, 0, 0], [93948, 91926, 91543, 71809, 0, 0, 0, 0],
                                 [124123, 115145, 61751, 48207, 0, 0, 0, 0], [131564, 38921, 107704, 71037, 0, 0, 0, 0],
                                 [41981, 158583, 128504, 20158, 0, 0, 0, 0], [39745, 74402, 160819, 74260, 0, 0, 0, 0],
                                 [12156, 161993, 101991, 73086, 0, 0, 0, 0]],
        'total_analyzed': 6895840
    },
    8: {
        'cache_slice_sizes': [2097152, 4194304, 8388608, 16777216, 33554432, 67108864, 134217728],
        'inter_node_misses': [198118, 197809, 197809, 197809, 197809, 197809, 197809],
        'way_partition_misses': [332224, 260938, 198118, 197809, 197809, 197809, 197809],
        'inter_intra_misses': [198118, 197809, 197809, 197809, 197809, 197809, 197809],
        'inter_node_accesses': [[46607, 47341, 46927, 44999, 44250, 47293, 36449, 35360],
                                [61000, 63123, 63866, 51279, 32948, 28803, 27677, 20530],
                                [66462, 65102, 22873, 16048, 57661, 50043, 38878, 32159],
                                [10729, 31252, 94474, 64109, 120835, 7669, 13230, 6928],
                                [38016, 1729, 65934, 8468, 3965, 156854, 62570, 11690],
                                [6751, 5405, 156752, 5241, 32994, 68997, 4067, 69019],
                                [8025, 2341, 826, 61554, 4131, 159652, 101165, 11532]],
        'way_partition_accesses': [[43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021]],
        'inter_intra_accesses': [[46607, 47341, 46927, 44999, 44250, 47293, 36449, 35360],
                                 [61000, 63123, 63866, 51279, 32948, 28803, 27677, 20530],
                                 [66462, 65102, 22873, 16048, 57661, 50043, 38878, 32159],
                                 [10729, 31252, 94474, 64109, 120835, 7669, 13230, 6928],
                                 [38016, 1729, 65934, 8468, 3965, 156854, 62570, 11690],
                                 [6751, 5405, 156752, 5241, 32994, 68997, 4067, 69019],
                                 [8025, 2341, 826, 61554, 4131, 159652, 101165, 11532]],
        'total_analyzed': 6895840
    },
}

OLD_v1 = {
    1: {
        'cache_slice_sizes': [2097152, 4194304, 8388608, 16777216, 33554432, 67108864, 134217728],
        'inter_node_misses': [343223, 332224, 260938, 198118, 197809, 197809, 197809],
        'way_partition_misses': [345798, 344893, 340140, 311702, 258988, 247851, 199398],
        'inter_intra_misses': [343223, 332224, 260938, 198118, 197809, 197809, 197809],
        'inter_node_accesses': [[349226], [349226], [349226], [349226], [349226], [349226], [349226]],
        'way_partition_accesses': [[43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021]],
        'inter_intra_accesses': [[349226, 0, 0, 0, 0, 0, 0, 0], [349226, 0, 0, 0, 0, 0, 0, 0],
                                 [349226, 0, 0, 0, 0, 0, 0, 0], [349226, 0, 0, 0, 0, 0, 0, 0],
                                 [349226, 0, 0, 0, 0, 0, 0, 0], [349226, 0, 0, 0, 0, 0, 0, 0],
                                 [349226, 0, 0, 0, 0, 0, 0, 0]],
        'total_analyzed': 6895840
    },
    2: {
        'cache_slice_sizes': [2097152, 4194304, 8388608, 16777216, 33554432, 67108864, 134217728],
        'inter_node_misses': [343223, 332224, 260938, 198118, 197809, 197809, 197809],
        'way_partition_misses': [344661, 343468, 301366, 262941, 209468, 200304, 197817],
        'inter_intra_misses': [332224, 260938, 198118, 197809, 197809, 197809, 197809],
        'inter_node_accesses': [[174408, 174818], [174408, 174818], [174408, 174818], [174408, 174818],
                                [174408, 174818], [174408, 174818], [174408, 174818]],
        'way_partition_accesses': [[43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021]],
        'inter_intra_accesses': [[174233, 174993, 0, 0, 0, 0, 0, 0], [185491, 163735, 0, 0, 0, 0, 0, 0],
                                 [185874, 163352, 0, 0, 0, 0, 0, 0], [239268, 109958, 0, 0, 0, 0, 0, 0],
                                 [170485, 178741, 0, 0, 0, 0, 0, 0], [200564, 148662, 0, 0, 0, 0, 0, 0],
                                 [114147, 235079, 0, 0, 0, 0, 0, 0]],
        'total_analyzed': 6895840
    },
    4: {
        'cache_slice_sizes': [2097152, 4194304, 8388608, 16777216, 33554432, 67108864, 134217728],
        'inter_node_misses': [343223, 332224, 260938, 198118, 197809, 197809, 197809],
        'way_partition_misses': [343327, 322640, 241698, 204367, 197857, 197809, 197809],
        'inter_intra_misses': [260938, 198118, 197809, 197809, 197809, 197809, 197809],
        'inter_node_accesses': [[87312, 87141, 87096, 87677], [87312, 87141, 87096, 87677],
                                [87312, 87141, 87096, 87677], [87312, 87141, 87096, 87677],
                                [87312, 87141, 87096, 87677], [87312, 87141, 87096, 87677],
                                [87312, 87141, 87096, 87677]],
        'way_partition_accesses': [[43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021]],
        'inter_intra_accesses': [[90857, 94634, 83376, 80359, 0, 0, 0, 0], [93948, 91926, 91543, 71809, 0, 0, 0, 0],
                                 [124123, 115145, 61751, 48207, 0, 0, 0, 0], [131564, 38921, 107704, 71037, 0, 0, 0, 0],
                                 [41981, 158583, 128504, 20158, 0, 0, 0, 0], [39745, 74402, 160819, 74260, 0, 0, 0, 0],
                                 [12156, 161993, 101991, 73086, 0, 0, 0, 0]],
        'total_analyzed': 6895840
    },
    8: {
        'cache_slice_sizes': [2097152, 4194304, 8388608, 16777216, 33554432, 67108864, 134217728],
        'inter_node_misses': [343223, 332224, 260938, 198118, 197809, 197809, 197809],
        'way_partition_misses': [332224, 260938, 198118, 197809, 197809, 197809, 197809],
        'inter_intra_misses': [198118, 197809, 197809, 197809, 197809, 197809, 197809],
        'inter_node_accesses': [[43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021]],
        'way_partition_accesses': [[43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021],
                                   [43759, 43693, 43575, 43656, 43553, 43448, 43521, 44021]],
        'inter_intra_accesses': [[46607, 47341, 46927, 44999, 44250, 47293, 36449, 35360],
                                 [61000, 63123, 63866, 51279, 32948, 28803, 27677, 20530],
                                 [66462, 65102, 22873, 16048, 57661, 50043, 38878, 32159],
                                 [10729, 31252, 94474, 64109, 120835, 7669, 13230, 6928],
                                 [38016, 1729, 65934, 8468, 3965, 156854, 62570, 11690],
                                 [6751, 5405, 156752, 5241, 32994, 68997, 4067, 69019],
                                 [8025, 2341, 826, 61554, 4131, 159652, 101165, 11532]],
        'total_analyzed': 6895840
    },
}

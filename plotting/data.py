inter_vs_cluster_way_partitioning_vs_inter_intra = {
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

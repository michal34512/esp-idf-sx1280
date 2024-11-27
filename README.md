# sx1280 driver for esp-idf

## Credits
This driver is based on sx1280 library which can be found [here]( https://www.semtech.com/products/wireless-rf/lora-connect/sx1280)

## Datarates
Here are the data rates achieved with this library. Notice how the data rate changes based on the payload size

The data rate is linearly dependent on the payload size, which indicates that LoRa always transmits 256 bytes regardless of the configured payload size. After 239 bytes, a drop occurs — likely caused by a buffer overflow in LoRa (since additional data is transmitted alongside the payload). Assuming the payload is always 256 bytes, the theoretical data rate in practice is approximately 204 kbps (the manufacturer specifies 203 kbps under these settings)
```
Data Rate Table [bps] for a 100-byte payload:

                    SF5           SF6           SF7           SF8           SF9         SF10         SF11         SF12
Bandwidth
BW_0200    20008.298828  13333.110352   7272.661621   4444.419922   2499.992188  1379.307983   629.920776   349.344849
BW_0400    39998.019531  26665.787109  13333.113281   8888.791016   4999.969238  2758.611328  1249.998047   695.651550
BW_0800    79992.085938  39998.019531  26665.787109  15999.682617   8898.678711  5333.297852  2499.992188  1379.307983
BW_1600    79992.085938  79992.085938  39998.019531  26665.787109  15999.682617  9999.875977  4999.969238  2758.611328
Data Rate Table [bps] for a 200-byte payload:

                     SF5           SF6           SF7           SF8           SF9          SF10         SF11         SF12
Bandwidth
BW_0200     22862.560547  13333.221680   7999.960449   4705.868652   2666.662354   1467.888550   669.455811   371.229614
BW_0400     39999.007812  26666.226562  15999.841797   9411.709961   5333.315918   2909.085693  1333.332275   740.740417
BW_0800     79996.039062  53331.574219  31999.365234  17777.582031  10006.192383   5714.265625  2666.662354  1481.480103
BW_1600    159984.171875  79996.039062  53331.574219  31999.365234  19999.751953  11428.490234  5333.315918  2962.957520
Data Rate Table [bps] for a 229-byte payload:

                     SF5           SF6           SF7           SF8           SF9          SF10         SF11         SF12
Bandwidth
BW_0200     22947.558594  14091.223633   8326.894531   4697.315430   2694.078125   1489.418701   676.012268   374.641357
BW_0400     45788.554688  26167.689453  16653.031250   9159.542969   5388.076660   2954.791016  1347.048950   747.752075
BW_0800     91554.218750  45788.554688  30528.246094  18318.167969  10177.212891   5909.486816  2694.078125  1489.418701
BW_1600    183009.671875  91554.218750  61046.316406  36632.671875  20353.294922  11449.284180  5388.076660  2954.791016
Data Rate Table [bps] for a 230-byte payload:

                    SF5           SF6           SF7           SF8           SF9          SF10         SF11         SF12
Bandwidth
BW_0200    23047.767578  14152.757812   8363.256836   4717.827637   2666.628174   1483.859009   676.468079   374.744629
BW_0400    45988.503906  26281.958984  16725.751953   9199.540039   5256.992676   2967.694092  1352.931274   747.964478
BW_0800    91954.023438  45988.503906  30661.556641  18398.160156  10221.654297   5935.292480  2705.842529  1495.922729
BW_1600    96791.164062  91954.023438  61312.894531  36792.640625  20442.173828  11499.281250  5411.605469  2967.694092
```

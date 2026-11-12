import struct,sys
d=open(sys.argv[1],'rb').read(512)
tag,legacy=struct.unpack_from('<Ii',d,0); o=8
if legacy!=-4: o+=4
ue4,ue5=struct.unpack_from('<ii',d,o); o+=8
o+=4 # licensee
# skip custom versions after savedhash? UE5.1+: SavedHash(20)+TotalHeaderSize
print(hex(tag),legacy,ue4,ue5)

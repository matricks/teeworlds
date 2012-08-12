
# remove old images
for i in data/menuimages/*.gray.png;
do rm $i;
done

# create new gray versions
for i in data/menuimages/*.png;
do cp $i ${i%%.png}.gray.png;
done

for i in data/menuimages/*.gray.png;
do ./gray $i;
done
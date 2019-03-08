Build with buildRoot
##########################
03.06, 2019


git clone https://github.com/linux4sam/buildroot-at91.git

git clone https://github.com/linux4sam/buildroot-external-microchip.git

cd buildroot-external-microchip/
git tag | grep linux4sam

git checkout linux4sam_6.0 -b buildroot-external-microchip-linux4sam_6.0

git checkout linux4sam_6.0 -b buildroot-at91-linux4sam_6.0


BR2_EXTERNAL=../buildroot-external-microchip/ make sama5d2_xplained_headless_defconfig

BR2_EXTERNAL=../buildroot-external-microchip/ make sama5d2_xplained_headless_defconfig
 
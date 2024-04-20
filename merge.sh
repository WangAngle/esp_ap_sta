BUILD_PATH=./build
PROJECT_NAME=WIFI_APSTA
FW_FILE=$PROJECT_NAME.bin

OUT_NAME=$PROJECT_NAME\_ESP32C3
OUT_PATH=firmware/
BURN_FILE=$OUT_NAME.bin
APP_FILE=$OUT_NAME-APP.bin

esptool.py --chip esp32c3  merge_bin -o $BURN_FILE \
0x0 $BUILD_PATH/bootloader/bootloader.bin \
0x8000 $BUILD_PATH/partition_table/partition-table.bin \
0x10000 $BUILD_PATH/$FW_FILE

if [ -f $BUILD_PATH/$FW_FILE ]
then 
	cp $BUILD_PATH/$FW_FILE ./$APP_FILE

	rm -rf 			$OUT_PATH
	mkdir			$OUT_PATH
	mv $BURN_FILE	$OUT_PATH
	mv $APP_FILE	$OUT_PATH

else
    echo "$FW_FILE is not exist in $BUILD_PATH , please make build and generate $FW_FILE"
fi

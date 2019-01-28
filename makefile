WARNS=-Wall -ansi -pedantic -std=c89
LEVEL=-O0
FRAMEWORK_PATH=/Library/Frameworks
FRAMEWORK_FLAGS=-F/Library/Frameworks -framework
ENTRY=src/main.c
APP_NAME=Robovac

all:
	gcc $(ENTRY) $(LEVEL) $(WARNS) $(FRAMEWORK_FLAGS) SDL2 -o $(APP_NAME)

run:
	gcc $(ENTRY) $(LEVEL) $(WARNS) $(FRAMEWORK_FLAGS) SDL2 -o $(APP_NAME) && ./$(APP_NAME)

undef:
	gcc $(ENTRY) -O0 -fsanitize=undefined $(WARNS) $(FRAMEWORK_FLAGS) SDL2 -o $(APP_NAME) && ./$(APP_NAME)

debug:
	gcc $(ENTRY) -O0 $(WARNS) $(FRAMEWORK_FLAGS) SDL2 -g -o $(APP_NAME) && lldb $(APP_NAME)

package_app:
	mkdir -p "./build/$(APP_NAME).app"/Contents/{MacOS,Resources}
	cp -R "$(FRAMEWORK_PATH)/SDL2.framework" "./build/$(APP_NAME).app/Contents/Resources/"
	cp Info.plist "./build/$(APP_NAME).app/Contents/"
	cp -R "./images" "./build/$(APP_NAME).app/Contents/MacOS/"
	sed -e "s/APP_NAME/$(APP_NAME)/g" -i "" "./build/$(APP_NAME).app/Contents/Info.plist"
	cp ./$(APP_NAME) "./build/$(APP_NAME).app/Contents/MacOS/$(APP_NAME)"

clean:
	rm -f $(APP_NAME) && rm -f -rf $(APP_NAME).dSYM && rm -f -rf build
	

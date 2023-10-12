.PHONY: clean All

All:
	@echo "----------Building project:[ MCServerStatusBot - Release ]----------"
	@cd "MCServerStatusBot" && "$(MAKE)" -f  "MCServerStatusBot.mk"
clean:
	@echo "----------Cleaning project:[ MCServerStatusBot - Release ]----------"
	@cd "MCServerStatusBot" && "$(MAKE)" -f  "MCServerStatusBot.mk" clean

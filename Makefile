.PHONY: clean All

All:
	@echo "----------Building project:[ MCServerStatusBot - Debug ]----------"
	@cd "MCServerStatusBot" && "$(MAKE)" -f  "MCServerStatusBot.mk"
clean:
	@echo "----------Cleaning project:[ MCServerStatusBot - Debug ]----------"
	@cd "MCServerStatusBot" && "$(MAKE)" -f  "MCServerStatusBot.mk" clean

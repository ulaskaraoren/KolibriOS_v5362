if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("MenuetOS.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "4.MenuetOS.1.skn")

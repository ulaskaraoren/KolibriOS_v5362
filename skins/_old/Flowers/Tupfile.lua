if tup.getconfig("NO_FASM") ~= "" then return end
tup.rule("Flowers.asm", 'fasm "%f" "%o" ' .. tup.getconfig("KPACK_CMD"), "Flowers.skn")

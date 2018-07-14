local function main()
    local f, err = io.open("../dxx.log", "w");
    
    if f then
      
      for i = 1, 10, 1 do
        f:write(i .. "\r\n");
      end;
      
      f:close();
    end;
    
    f, err = io.open("../dxx.log", "r");
    
    if f then
      
      for l in f:lines() do
        print(l);
      end;
      
      f:close();
    end;
    
end;

main();

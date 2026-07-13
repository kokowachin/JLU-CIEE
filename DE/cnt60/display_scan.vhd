library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;   -- 用于计数器加法

entity display_scan is
    port (
        clk_scan : in  std_logic;          -- 扫描时钟（建议频率 500Hz~1kHz）
        rst      : in  std_logic;          -- 异步复位（高有效）
        an       : out std_logic_vector(3 downto 0); -- 位选（低有效，an(0)=个位，an(3)=千位）
        sel      : out std_logic_vector(1 downto 0)  -- 当前选中位编号（0~3）
    );
end display_scan;

architecture bh of display_scan is
    signal sel_reg : std_logic_vector(1 downto 0);  -- 2位计数器
begin
    process(clk_scan, rst)
    begin
        if rst = '1' then
            sel_reg <= "00";
        elsif rising_edge(clk_scan) then
            sel_reg <= sel_reg + 1;       -- 自动循环 00→01→10→11→00...
        end if;
    end process;
    
    -- 位选输出：低有效，根据 sel_reg 译码
    with sel_reg select
        an <= "0001" when "00",   -- 个位
              "0010" when "01",   -- 十位
              "0100" when "10",   -- 百位
              "1000" when "11",   -- 千位
              "0000" when others; -- 安全值
    
    sel <= sel_reg;
end bh;
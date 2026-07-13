library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity clk_div is
    generic (
        DIV_RATIO : integer := 2048   -- 分频系数：50MHz -> 1kHz (50M/(2*25000)=1k)
    );
    port (
        clk_in  : in  std_logic;       -- 输入时钟（如50MHz）
        rst     : in  std_logic;       -- 异步复位（高有效）
        clk_out : out std_logic        -- 输出分频时钟（占空比50%）
    );
end clk_div;

architecture bh of clk_div is
    signal cnt   : integer range 0 to DIV_RATIO - 1;
    signal clk_reg : std_logic;
begin
    process(clk_in, rst)
    begin
        if rst = '1' then
            cnt     <= 0;
            clk_reg <= '0';
        elsif rising_edge(clk_in) then
            if cnt = DIV_RATIO - 1 then
                cnt     <= 0;
                clk_reg <= not clk_reg;
            else
                cnt <= cnt + 1;
            end if;
        end if;
    end process;
    
    clk_out <= clk_reg;
end bh;
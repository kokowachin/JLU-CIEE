library ieee;
use ieee.std_logic_1164.all;
-- 注意：以下两个库在整数运算中并非必需，但保留以保持与您的工程兼容
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

entity cnt is
    port (
        clk     : in  std_logic;          -- 系统时钟（例如 2.048MHz 或 50MHz）
        en      : in  std_logic;          -- 使能控制（高有效）
        get1    : in  std_logic;          -- 按键脉冲（已消抖，上升沿有效）
        co      : out std_logic;          -- 进位输出（计数到 59 时产生一个脉冲）
        bcd_uni : out std_logic_vector(3 downto 0);  -- 个位 BCD 码
        bcd_ten : out std_logic_vector(3 downto 0)   -- 十位 BCD 码
    );
end cnt;

architecture bh of cnt is
    signal uni : integer range 0 to 9 := 0;   -- 个位计数器
    signal ten : integer range 0 to 5 := 0;   -- 十位计数器
begin
    -- 计数进程：在时钟上升沿，使能且按键脉冲有效时计数
    process(clk)
    begin
        if rising_edge(clk) then
            if en = '1' and get1 = '1' then
                if uni = 9 then
                    uni <= 0;
                    if ten = 5 then
                        ten <= 0;
                    else
                        ten <= ten + 1;
                    end if;
                else
                    uni <= uni + 1;
                end if;
            end if;
        end if;
    end process;

    -- 进位输出：当计数值为 59 且使能及按键脉冲同时有效时输出高电平（组合逻辑，产生一个时钟周期脉冲）
    co <= '1' when (uni = 9 and ten = 5 and en = '1' and get1 = '1') else '0';

    -- 个位 BCD 映射（完全仿照 cnt6）
    with uni select
        bcd_uni <= "0000" when 0,
                   "0001" when 1,
                   "0010" when 2,
                   "0011" when 3,
                   "0100" when 4,
                   "0101" when 5,
                   "0110" when 6,
                   "0111" when 7,
                   "1000" when 8,
                   "1001" when 9;

    -- 十位 BCD 映射（完全仿照 cnt10，只用到 0~5）
    with ten select
        bcd_ten <= "0000" when 0,
                   "0001" when 1,
                   "0010" when 2,
                   "0011" when 3,
                   "0100" when 4,
                   "0101" when 5;
end bh;
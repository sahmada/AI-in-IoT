// import solidity
pragma solidity ^0.4.0;

contract Inheritance {
    address owner;
    bool deceased;
    uint money;
    
    constructor() public payable {
        owner = msg.sender;
        money = msg.value;
        deceased = false;
    }
    
    modifier oneOwner {
        require (msg.sender == owner);
        _;
    }
    
    modifier isDeceased {
        require (deceased == true);
        _;
    }
    
    address[] wallets;
    
    mapping (address => uint) inheritance;

}
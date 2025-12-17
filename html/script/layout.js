// layout.js - 统一布局脚本

// DOM加载完成后执行
document.addEventListener('DOMContentLoaded', function() {
    // 初始化主题（在DOM加载完成后立即执行，不等待header和footer加载）
    initTheme();
    
    // 先加载用户信息脚本
    loadUserInfoScript();
    
    // 加载页眉
    loadHeader();
    
    // 加载页脚
    loadFooter();
    
    // 初始化通用功能
    initCommonFunctions();
});

// 加载用户信息脚本
function loadUserInfoScript() {
    const userInfoScript = document.createElement('script');
    userInfoScript.src = 'script/user_info.js';
    userInfoScript.onload = function() {
        console.log('User info script loaded successfully');
    };
    userInfoScript.onerror = function() {
        console.error('Failed to load user info script');
    };
    document.head.appendChild(userInfoScript);
}

// 加载页眉
function loadHeader() {
    fetch('header.html')
        .then(response => {
            if (!response.ok) {
                throw new Error('Network response was not ok');
            }
            return response.text();
        })
        .then(html => {
            const headerContainer = document.getElementById('header-container');
            if (headerContainer) {
                headerContainer.innerHTML = html;
                
                // 页眉加载完成后，重新执行用户信息初始化逻辑
                // 确保登录状态的用户图标能正确显示
                if (typeof initUserInfo === 'function') {
                    initUserInfo();
                }
            }
        })
        .catch(error => {
            console.error('Error loading header:', error);
        });
}

// 加载页脚
function loadFooter() {
    fetch('footer.html')
        .then(response => {
            if (!response.ok) {
                throw new Error('Network response was not ok');
            }
            return response.text();
        })
        .then(html => {
            const footerContainer = document.getElementById('footer-container');
            if (footerContainer) {
                footerContainer.innerHTML = html;
                
                // 页脚加载完成后重新初始化主题切换功能，确保事件绑定正确
                initTheme();
            }
        })
        .catch(error => {
            console.error('Error loading footer:', error);
        });
}

// 初始化通用功能
function initCommonFunctions() {
    // 搜索功能
    const searchInput = document.querySelector('.search-box input');
    if (searchInput) {
        searchInput.addEventListener('keypress', function(e) {
            if (e.key === 'Enter') {
                performSearch();
            }
        });
    }
    
    const searchBtn = document.querySelector('.search-box button');
    if (searchBtn) {
        searchBtn.addEventListener('click', performSearch);
    }
    
    // 移动端菜单切换逻辑
    const mobileToggle = document.querySelector('.mobile-toggle');
    if (mobileToggle) {
        mobileToggle.addEventListener('click', toggleMenu);
    }
}

// 搜索功能实现
function performSearch() {
    const input = document.querySelector('.search-box input');
    if (input) {
        const keyword = input.value.trim();
        if (keyword) {
            window.location.href = `search.html?keyword=${encodeURIComponent(keyword)}`;
        } else {
            alert('请输入搜索关键词');
        }
    }
}

// 主题切换功能
const THEME_STORAGE_KEY = 'purecpp_theme';
const htmlRoot = document.documentElement;

// 初始化主题
function initTheme() {
    const themeToggleBtn = document.getElementById('themeToggleBtn');
    if (themeToggleBtn) {
        const savedTheme = localStorage.getItem(THEME_STORAGE_KEY);
        const isDarkMode = savedTheme === 'dark' || (!savedTheme && window.matchMedia('(prefers-color-scheme: dark)').matches);
        themeToggleBtn.checked = isDarkMode;
        htmlRoot.classList.add('manual-theme');
        
        // 确保只应用一个主题类
        htmlRoot.classList.remove('dark', 'light');
        htmlRoot.classList.add(isDarkMode ? 'dark' : 'light');
        
        // 绑定主题切换事件
        themeToggleBtn.addEventListener('change', toggleTheme);
    }
}

// 切换主题
function toggleTheme() {
    const themeToggleBtn = document.getElementById('themeToggleBtn');
    if (themeToggleBtn) {
        const isDarkMode = themeToggleBtn.checked;
        
        // 确保只应用一个主题类
        htmlRoot.classList.remove('dark', 'light');
        htmlRoot.classList.add(isDarkMode ? 'dark' : 'light');
        
        localStorage.setItem(THEME_STORAGE_KEY, isDarkMode ? 'dark' : 'light');
    }
}

// 移动端菜单切换
function toggleMenu() {
    const navMenu = document.getElementById('nav-menu');
    if (navMenu) {
        navMenu.classList.toggle('active');
        
        // 切换图标
        const icon = document.querySelector('.mobile-toggle i');
        if (icon) {
            if (navMenu.classList.contains('active')) {
                icon.classList.remove('fa-bars');
                icon.classList.add('fa-times');
            } else {
                icon.classList.remove('fa-times');
                icon.classList.add('fa-bars');
            }
        }
    }
}
/**
 * Copyright (c) 2017-present, Facebook, Inc.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

/* List of projects/orgs using your project for the users page */

const users = [
  {	
    caption: 'AppImageUpdater',
    image: '/AppImageUpdaterBridge/img/AppImageUpdater.png',
    infoLink: 'https://github.com/antony-jr/AppImageUpdater',
    pinned: true,
  },
  {
    caption: 'e2designer',
    image: '/AppImageUpdaterBridge/img/e2designer.png',
    infoLink: 'https://gitlab.com/technic93/e2designer',
    pinned: true,
  }
];

const siteConfig = {
  title: 'AppImage Updater Bridge' /* title for your website */,
  tagline: 'A Powerfull Bridge to AppImage Update Mechanism.',
  url: 'https://antony-jr.github.io' /* your website url */,
  baseUrl: '/AppImageUpdaterBridge/' /* base url for your project */,
  projectName: 'AppImageUpdaterBridge',
  headerLinks: [
    {doc: 'Installation', label: 'Docs'},
    {page: 'help', label: 'Help'},
    {blog: false, label: 'Blog'},
  ],
  users,
  /* path to images for header/footer */
  headerIcon: 'img/AppImageUpdaterBridge.png',
  footerIcon: 'img/AppImageUpdaterBridge.png',
  favicon: 'img/favicon.png',
  /* colors for website */
  colors: {
    primaryColor: '#709cb7',
    secondaryColor: '#1a8cff',
  },
  // This copyright info is used in /core/Footer.js and blog rss/atom feeds.
  copyright:
    'Copyright © ' +
    new Date().getFullYear() +
    ' Antony Jr.',
  organizationName: 'antony-jr', // or set an env variable ORGANIZATION_NAME
  highlight: {
    // Highlight.js theme to use for syntax highlighting in code blocks
    theme: 'default',
  },
  scripts: ['https://buttons.github.io/buttons.js'],
  // You may provide arbitrary config keys to be used as needed by your template.
  repoUrl: 'https://github.com/antony-jr/AppImageUpdaterBridge',
};

module.exports = siteConfig;
